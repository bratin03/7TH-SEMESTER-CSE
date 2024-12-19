    		+------------------------------------------------------------------------+
    		|                                CS60203                                 |
    		| Assignment 2 - Introduction to PintOS and improving threads in PintOS  |
    		|                            Design Documnet                             |
    		+------------------------------------------------------------------------+

---- GROUP ----

> > Fill in the names and email addresses of your group members.

Bratin Mondal <mondalbratin2003@kgpian.iitkgp.ac.in>

---- PRELIMINARIES ----

> > If you have any preliminary comments on your submission, notes for the
> > TAs, or extra credit, please give them here.

> > Please cite any offline or online sources you consulted while
> > preparing your submission, other than the Pintos documentation, course
> > text, lecture notes, and course staff.

    		                              ALARM CLOCK
    		                              ===========

---- DATA STRUCTURES ----

> > A1: Copy here the declaration of each new or changed `struct` or `struct` member, global or static variable, `typedef`, or
> > enumeration. Identify the purpose of each in 25 words or less.

1. In thread.h, added a new member `wakeup_time` to the `struct thread` to store the time at which the thread should be woken up. When a thread is put to sleep, this value is set to the current time plus the sleep duration.

```c
struct thread
{
   /* Owned by thread.c. */
   tid_t tid;                 /* Thread identifier. */
   enum thread_status status; /* Thread state. */
   char name[16];             /* Name (for debugging purposes). */
   uint8_t *stack;            /* Saved stack pointer. */
   int priority;              /* Priority. */
   struct list_elem allelem;  /* List element for all threads list. */

   /* Shared between thread.c and synch.c. */
   struct list_elem elem; /* List element. */

   /* ------------------------------ START OF CODE ADDED ------------------------------ */
   int64_t wakeup_time; /* Time to wake up. */
   /* ------------------------------- END OF CODE ADDED ------------------------------- */

#ifdef USERPROG
   /* Owned by userprog/process.c. */
   uint32_t *pagedir; /* Page directory. */
#endif

   /* Owned by thread.c. */
   unsigned magic; /* Detects stack overflow. */
};
```

2. In thread.c defined a pointer to a struct `thread` named `wakeup_thread` to store pointer to the thread that is responsible for waking up sleeping threads. This thread is used to wake up sleeping threads in the timer interrupt handler.

```c
static struct thread *wakeup_thread; /* Thread to wake up sleeping threads */
```

3. In timer.h added a new global int64_t variable `next_wake` to store the time at which the wakeup_thread should be unblocked. This is used to wake up the wakeup_thread at the correct time. It is set to the minimum of the wakeup time of the all the sleeping threads.

```c
int64_t next_wake; // The next wake up time for the wakeup thread
```

Also in timer.h, added a new list `sleepers` to store the threads that are currently sleeping. This list is used to keep track of the threads that are sleeping and need to be woken up. The threads are sorted in the list sorted by their wakeup time.

```c
struct list sleepers; // List of sleeping threads
```

In timer.h, added a new macro `MAX_WAKEUP_COUNT` to store the maximum number of threads that can be woken up in one iteration holding the `sleepers_lock`. This is to ensure that the lock is not held for too long and other threads who are trying to sleep are not blocked for too long by the wakeup thread.

```c
#define MAX_WAKEUP_COUNT 16 // Maximum number of threads that can be woken up in one iteration holding the sleepers_lock
```

4. In timer.c, added a struct lock `sleepers_lock` to protect the `sleepers` list from concurrent access by the wakeup thread and other threads that are trying to sleep.

```c
static struct lock sleepers_lock; // Lock for the sleepers list
```

---- ALGORITHMS ----

> > A2: Briefly describe what happens in a call to timer_sleep(),
> > including the effects of the timer interrupt handler.

When a thread calls `timer_sleep()`, it is first checked for if the parameter `ticks` is a positive value. If it is not, the function returns immediately. Otherwise, it first disables interrupts using `intr_disable()`. It then acquires a lock on `sleepers lock` to protect the `sleepers` list from concurrent access. It then calculates the target wakeup time by adding the current time to the number of ticks the thread wants to sleep and stores this value in the `wakeup_time` field of the thread. The thread is then added to the `sleepers` list using the `list_insert_ordered()` function which inserts the thread in the list in sorted order by wakeup time. It is also checked if the new thread added has the minimum wakeup time and if so, the `next_wake` variable is updated to the wakeup time of the new thread. The thread then gives up the lock on `sleepers_lock`.The thread is blocked using `thread_block()` which changes the status of the thread to `THREAD_BLOCKED` and calls the `schedule()` function to switch to the next thread. On return from the `thread_block()` function, interrupts are set to the previous state using `intr_set_level()`.

When the timer interrupt handler is called, it increments the `ticks` variable and then calls the `thread_tick()` function. The `thread_tick()` function checks if the current time is greater or equal to `next_wake`, if so the `wakeup_thread` is unblocked. It also checks if the current thread has exhausted its time slice and if so, calls the `intr_yield_on_return()` function to yield the CPU to the next thread. It is also checked here if the current thread has exhausted its time slice and if so, calls the `intr_yield_on_return()` function to yield the CPU to the next thread. The interrupt handler returns.

Next whenever the `wakeup_thread` is scheduled, it calls the `timer_wakeup` function. Another function named `_timer_wakeup` function is called by the `timer_wakeup` function. The `_timer_wakeup` function takes as parameter the maximum number of threads (Currently set to 16) that can be woken up in one iteration holding the `sleepers_lock`. It then acquires the lock on `sleepers_lock` and iterates over the `sleepers` list and wakes up the threads whose wakeup time has passed. This is done by checking if the current time is greater than or equal to the `wakeup_time` of the thread. If the condition is true, the thread is removed from the `sleepers` list and unblocked using the `thread_unblock()` function. The `thread_unblock()` function changes the status of the thread to `THREAD_READY` and adds it to the ready list. The iteration continues until the `sleepers` list is empty or the wakeup time of the first thread currently in the list is greater than the current time or the maximum number of threads that can be woken up in one iteration holding the `sleepers_lock` is reached. The lock is then released by `_timer_wakeup`. The `_timer_wakeup` function returns to the `timer_wakeup` function a boolean value indicating if more threads need to be woken up. The `timer_wakeup` function calls the `_timer_wakeup` function in a loop until the `_timer_wakeup` function returns false. On return from the `timer_wakeup` function, the `wakeup_thread` blocks itself using `thread_block()` waiting for the next wakeup time.

> > A3: What steps are taken to minimize the amount of time spent in
> > the timer interrupt handler?

On call to `timer_sleep()`, a thread is blocked and added to a `sleepers` list in sorted order by wakeup time. This means the thread that should wake up the soonest is placed at the front of the list and we always have the next wakeup time stored in the `next_wake` variable.

When the timer interrupt occurs, the interrupt handler checks the `next_wake` and wakes up the `wakeup_thread` if the current time is equal to the `next_wake`. The interrupt handler returns and it does not need to iterate over the `sleepers` list to wake up threads. So, the time spent in the interrupt handler is minimized by shifting the task of waking up threads to the `wakeup_thread` which is scheduled only when needed with high priority.

---- SYNCHRONIZATION ----

> > A4: How are race conditions avoided when multiple threads call
> > timer_sleep() simultaneously?

While adding a thread to the `sleeping_list`, a lock `sleepers_lock` is acquired to protect the `sleepers` list from concurrent access by the wakeup thread and other threads that are trying to sleep. This ensures that only one thread can access the `sleepers` list at a time. The lock is released after the thread is added to the list. This prevents potential race conditions that could occur when multiple threads try to access the `sleepers` list simultaneously.

> > A5: How are race conditions avoided when a timer interrupt occurs
> > during a call to timer_sleep()?

Timer interrupts can occur at any time and can interrupt the `timer_sleep()` function while it is executing. But notably, the timer interrupt handler does not access the `sleepers` list directly and also does not modify the `next_wake` variable. The timer interrupt handler only reads the `next_wake` variable and wakes up the `wakeup_thread` if the current time is less than or equal to the `next_wake`. Further waking up of threads is done by the `wakeup_thread` which needs to acquire the `sleepers_lock` to access the `sleepers` list. This ensures that the timer interrupt handler does not interfere with the `timer_sleep()` function and avoids race conditions.

---- RATIONALE ----

> > A6: Why did you choose this design? In what ways is it superior to
> > another design you considered?

The current design using a wakeup thread to wake up sleeping threads minimizes the time spent in the timer interrupt handler by ensuring that threads are woken up only when needed which is checked by comparing the current time with the `next_wake` variable. This design is superior to another design where the timer interrupt handler would iterate over the `sleepers` list to wake up threads. In the other design, the timer interrupt handler would have to iterate over the entire list to wake up threads which would be inefficient. The current design ensures that the timer interrupt handler only wakes up the `wakeup_thread` which in turn wakes up the sleeping threads. The wakeup thread is given the highest priority possible to ensure that it is scheduled as soon as possible when needed. This design ensures that the time spent in the timer interrupt handler is minimized and the threads are woken up efficiently.

The choice of keeping the sleepers list sorted by wakeup time ensures that the wakeup thread can wake up threads efficiently by iterating over the list and waking up threads whose wakeup time has passed and avoiding unnecessary iterations. It is done at the cost of the time spent in the `timer_sleep()` function to keep the list sorted.

Another design considered was to fix the maximum number of threads that can be woken up in one iteration holding the `sleepers_lock`. This was done to ensure that the lock is not held for too long and other threads who are trying to sleep are not blocked for too long by the wakeup thread. This design ensures that the wakeup thread does not hold the lock for too long and other threads are not blocked for too long. This design is superior to another design where the wakeup thread would iterate over the entire list to wake up threads. In the other design, the wakeup thread would hold the lock for a long time and other threads would be blocked on the lock for a long time which would be inefficient since we could have directly put them to sleep instead of blocking them on the lock.
