/*
    * Author: Bratin Mondal
    * Roll No: 21CS10016

    * Deparment of Computer Science and Engineering
    * Indian Institue of Technology, Kharagpur
*/

// #ifndef DEBUG
// #define DEBUG 1
// #endif

#include "devices/timer.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include "devices/pit.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"

/* See [8254] for hardware details of the 8254 timer chip. */

#if TIMER_FREQ < 19
#error 8254 timer requires TIMER_FREQ >= 19
#endif
#if TIMER_FREQ > 1000
#error TIMER_FREQ <= 1000 recommended
#endif

/* Number of timer ticks since OS booted. */
static int64_t ticks;

/* Number of loops per timer tick.
   Initialized by timer_calibrate(). */
static unsigned loops_per_tick;

/* ------------------------------ START OF CODE ADDED ------------------------------ */
static struct lock sleepers_lock; // Lock for the sleepers list
/* ------------------------------- END OF CODE ADDED ------------------------------- */

static intr_handler_func timer_interrupt;
static bool too_many_loops(unsigned loops);
static void busy_wait(int64_t loops);
static void real_time_sleep(int64_t num, int32_t denom);
static void real_time_delay(int64_t num, int32_t denom);

/* Sets up the timer to interrupt TIMER_FREQ times per second,
   and registers the corresponding interrupt. */
void timer_init(void)
{
  pit_configure_channel(0, 2, TIMER_FREQ);
  intr_register_ext(0x20, timer_interrupt, "8254 Timer");

  /* ------------------------------ START OF CODE ADDED ------------------------------ */
  list_init(&sleepers);      // Initialize the list of sleeping threads
  lock_init(&sleepers_lock); // Initialize the lock for the sleepers list
  next_wake = INT64_MAX;     // Initialize the next wake up time for the wakeup thread
  /* ------------------------------- END OF CODE ADDED ------------------------------- */

#ifdef DEBUG
  printf("timer_init called: next_wake = %lld\n", next_wake);
#endif
}

/* Calibrates loops_per_tick, used to implement brief delays. */
void timer_calibrate(void)
{
  unsigned high_bit, test_bit;

  ASSERT(intr_get_level() == INTR_ON);
  printf("Calibrating timer...  ");

  /* Approximate loops_per_tick as the largest power-of-two
     still less than one timer tick. */
  loops_per_tick = 1u << 10;
  while (!too_many_loops(loops_per_tick << 1))
  {
    loops_per_tick <<= 1;
    ASSERT(loops_per_tick != 0);
  }

  /* Refine the next 8 bits of loops_per_tick. */
  high_bit = loops_per_tick;
  for (test_bit = high_bit >> 1; test_bit != high_bit >> 10; test_bit >>= 1)
    if (!too_many_loops(loops_per_tick | test_bit))
      loops_per_tick |= test_bit;

  printf("%'" PRIu64 " loops/s.\n", (uint64_t)loops_per_tick * TIMER_FREQ);
}

/* Returns the number of timer ticks since the OS booted. */
int64_t
timer_ticks(void)
{
  enum intr_level old_level = intr_disable();
  int64_t t = ticks;
  intr_set_level(old_level);
  return t;
}

/* Returns the number of timer ticks elapsed since THEN, which
   should be a value once returned by timer_ticks(). */
int64_t
timer_elapsed(int64_t then)
{
  return timer_ticks() - then;
}

/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void timer_sleep(int64_t ticks)
{
  /* ------------------------------ START OF CODE ADDED ------------------------------ */
  if (ticks <= 0) // If the ticks is less than or equal to 0, return
    return;

  lock_acquire(&sleepers_lock);                                         // Acquire the lock for the sleepers list
  uint64_t start = timer_ticks();                                       // Get the current time
  int64_t wake_time = start + ticks;                                    // Calculate the wake up time
  struct thread *cur = thread_current();                                // Get the current thread
  cur->wakeup_time = wake_time;                                         // Set the wakeup time of the current thread
  list_insert_ordered(&sleepers, &cur->elem, thread_wakeup_less, NULL); // Insert the thread in the sleepers list in order of wakeup time

  // Check if the current thread is the earliest to wake up
  if (wake_time < next_wake)
  {
    next_wake = wake_time; // Set the next wake up time
  }
#ifdef DEBUG
  printf("timer_sleep called: thread %s | ticks = %lld | wakeup_time = %lld | next_wake = %lld\n", cur->name, ticks, cur->wakeup_time, next_wake);
#endif
  lock_release(&sleepers_lock); // Release the lock for the sleepers list

  enum intr_level old_level = intr_disable(); // Disable interrupts
  thread_block();            // Block the current thread
  intr_set_level(old_level); // Enable interrupts

  /* ------------------------------- END OF CODE ADDED ------------------------------- */
}

/* ------------------------------ START OF CODE ADDED ------------------------------ */
/*
Function to wake up sleeping threads by the wakeup thread upto max_wakeup_count threads in one iteration
*/
bool _timer_wakeup(int16_t max_wakeup_count)
{
#ifdef DEBUG
  printf("_timer_wakeup called\n");
#endif

  if (max_wakeup_count <= 0) // If the maximum wakeup count is less than or equal to 0, return
  {
    return false;
  }

  lock_acquire(&sleepers_lock); // Acquire the lock for the sleepers list
  int16_t wakeup_count = 0;     // Initialize the wakeup count
  // Iterate through the sleepers list until the wakeup time of the thread is greater than the current time or the list is empty
  while (!list_empty(&sleepers))
  {
    struct list_elem *e = list_begin(&sleepers);           // Get the first element of the sleepers list
    struct thread *t = list_entry(e, struct thread, elem); // Get the thread from the list element

    if (t->wakeup_time <= timer_ticks()) // If the wakeup time of the thread is less than or equal to the current time
    {
#ifdef DEBUG
      printf("timer_wakeup : thread %s is woken up\n", t->name);
#endif
      list_remove(e);    // Remove the thread from the sleepers list
      thread_unblock(t); // Unblock the thread
      wakeup_count++;    // Increment the wakeup count
    }
    else
    {
#ifdef DEBUG
      printf("timer_wakeup : thread %s is not woken up | wakeup_time = %lld\n", t->name, t->wakeup_time);
#endif
      next_wake = t->wakeup_time; // Set the next wake up time
      wakeup_count = -1;
      break;
    }
    if (wakeup_count == max_wakeup_count)
    {
      break;
    }
  }
  if (list_empty(&sleepers))
  {
    wakeup_count = -1;
#ifdef DEBUG
    printf("timer_wakeup : sleepers list is empty\n");
#endif
    next_wake = INT64_MAX; // If the sleepers list is empty, set the next wake up time to INT64_MAX
  }
  lock_release(&sleepers_lock); // Release the lock for the sleepers list
#ifdef DEBUG
  printf("timer_wakeup : wakeup_count = %d, next_wake = %lld", wakeup_count, next_wake);
  if (wakeup_count == -1)
  {
    printf(" No more threads to wake up\n");
  }
  else
  {
    printf(" More threads to wake up\n");
  }
#endif
  return wakeup_count != -1;
}

// Function to wake up sleeping threads by the wakeup thread. Don't wake up more than max_wakeup_count threads in one iteration
void timer_wakeup(void)
{
  while (_timer_wakeup(MAX_WAKEUP_COUNT))
    ;
}
/* ------------------------------- END OF CODE ADDED ------------------------------- */

/* Sleeps for approximately MS milliseconds.  Interrupts must be
   turned on. */
void timer_msleep(int64_t ms)
{
  real_time_sleep(ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts must be
   turned on. */
void timer_usleep(int64_t us)
{
  real_time_sleep(us, 1000 * 1000);
}

/* Sleeps for approximately NS nanoseconds.  Interrupts must be
   turned on. */
void timer_nsleep(int64_t ns)
{
  real_time_sleep(ns, 1000 * 1000 * 1000);
}

/* Busy-waits for approximately MS milliseconds.  Interrupts need
   not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_msleep()
   instead if interrupts are enabled. */
void timer_mdelay(int64_t ms)
{
  real_time_delay(ms, 1000);
}
/* Sleeps for approximately US microseconds.  Interrupts need not
   be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_usleep()
   instead if interrupts are enabled. */
void timer_udelay(int64_t us)
{
  real_time_delay(us, 1000 * 1000);
}

/* Sleeps execution for approximately NS nanoseconds.  Interrupts
   need not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_nsleep()
   instead if interrupts are enabled.*/
void timer_ndelay(int64_t ns)
{
  real_time_delay(ns, 1000 * 1000 * 1000);
}

/* Prints timer statistics. */
void timer_print_stats(void)
{
  printf("Timer: %" PRId64 " ticks\n", timer_ticks());
}

/* Timer interrupt handler. */
static void
timer_interrupt(struct intr_frame *args UNUSED)
{
  // printf("timer_interrupt called ticks = %lld\n", ticks);
  ticks++;
  thread_tick();
}

/* Returns true if LOOPS iterations waits for more than one timer
   tick, otherwise false. */
static bool
too_many_loops(unsigned loops)
{
  /* Wait for a timer tick. */
  int64_t start = ticks;
  while (ticks == start)
    barrier();

  /* Run LOOPS loops. */
  start = ticks;
  busy_wait(loops);

  /* If the tick count changed, we iterated too long. */
  barrier();
  return start != ticks;
}

/* Iterates through a simple loop LOOPS times, for implementing
   brief delays.

   Marked NO_INLINE because code alignment can significantly
   affect timings, so that if this function was inlined
   differently in different places the results would be difficult
   to predict. */
static void NO_INLINE
busy_wait(int64_t loops)
{
  while (loops-- > 0)
    barrier();
}

/* Sleep for approximately NUM/DENOM seconds. */
static void
real_time_sleep(int64_t num, int32_t denom)
{
  /* Convert NUM/DENOM seconds into timer ticks, rounding down.

        (NUM / DENOM) s
     ---------------------- = NUM * TIMER_FREQ / DENOM ticks.
     1 s / TIMER_FREQ ticks
  */
  int64_t ticks = num * TIMER_FREQ / denom;

  ASSERT(intr_get_level() == INTR_ON);
  if (ticks > 0)
  {
    /* We're waiting for at least one full timer tick.  Use
       timer_sleep() because it will yield the CPU to other
       processes. */
    timer_sleep(ticks);
  }
  else
  {
    /* Otherwise, use a busy-wait loop for more accurate
       sub-tick timing. */
    real_time_delay(num, denom);
  }
}

/* Busy-wait for approximately NUM/DENOM seconds. */
static void
real_time_delay(int64_t num, int32_t denom)
{
  /* Scale the numerator and denominator down by 1000 to avoid
     the possibility of overflow. */
  ASSERT(denom % 1000 == 0);
  busy_wait(loops_per_tick * num / 1000 * TIMER_FREQ / (denom / 1000));
}
