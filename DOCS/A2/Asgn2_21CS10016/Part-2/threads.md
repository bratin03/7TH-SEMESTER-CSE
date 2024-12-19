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

> > A1: Copy here the declaration of each new or changed `struct' or
`struct' member, global or static variable, `typedef', or
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
   int64_t wakeup_time; /* Time at which the thread should be woken up */
   /* ------------------------------ END OF CODE ADDED ------------------------------*/

#ifdef USERPROG
   /* Owned by userprog/process.c. */
   uint32_t *pagedir; /* Page directory. */
#endif

   /* Owned by thread.c. */
   unsigned magic; /* Detects stack overflow. */
};
```

2.  In timer.c, added a new list `sleeping_list` to store the threads that are currently sleeping. This list is used to keep track of the threads that are sleeping and need to be woken up. The threads are sorted in the list based on their wakeup time.

```c
static struct list sleeping_list; /* List of sleeping threads */
```

---- ALGORITHMS ----

> > A2: Briefly describe what happens in a call to timer_sleep(),
> > including the effects of the timer interrupt handler.

When a thread calls `timer_sleep()`, it is first checked for if the parameter `ticks` is a positive value. If it is not, the function returns immediately. Otherwise, interrupts are disabled using `intr_disable` and the target wakeup time is calculated by adding the current time to the number of ticks the thread wants to sleep and this value is stored in the `wakeup_time` field of the thread. The thread is then added to the `sleeping_list` using the `list_insert_ordered()` function which inserts the thread in the list in sorted order based on the `wakeup_time`. The thread is then blocked using `thread_block()` which changes the status of the thread to `THREAD_BLOCKED` and calls the `schedule()` function to switch to the next thread. On return from the `thread_block()` function, interrupts are set to the previous state using `intr_set_level()`.

When the timer interrupt handler is called, it increments the `ticks` variable and then calls the `thread_tick()` function. The `thread_tick()` function checks if the current thread has exhausted its time slice and if so, calls the `intr_yield_on_return()` function to yield the CPU to the next thread. On return from the `thread_tick()` function iterates over the `sleeping_list` and wakes up the threads whose wakeup time has passed. This is done by checking if the current time is greater than or equal to the `wakeup_time` of the thread. If the condition is true, the thread is removed from the `sleeping_list` and unblocked using the `thread_unblock()` function. The `thread_unblock()` function changes the status of the thread to `THREAD_READY` and adds it to the ready list. The iteration continues until the `sleeping_list` is empty or the wakeup time of the first thread currently in the list is greater than the current time. As mentioned in the problem statement, the current thread is not preempted in the middle of the `timer_sleep()` function. It essentially gurantees that the thread will sleep for at least the specified number of ticks.

> > A3: What steps are taken to minimize the amount of time spent in
> > the timer interrupt handler?

On a call to `timer_sleep()`, a thread is blocked and added to a `sleeping_list` in sorted order by wakeup time. This means the thread that should wake up the soonest is placed at the front of the list.

When the timer interrupt occurs, the interrupt handler checks the `sleeping_list` and wakes up any threads whose wakeup time has arrived. The handler only processes threads up to the first one whose wakeup time is still in the future. It does not iterate through the entire list unnecessarily.

By keeping the list sorted and stopping the iteration as soon as a thread's wakeup time is later than the current time, the time spent in the interrupt handler is minimized at a little cose of the time spent in the `timer_sleep()` function to keep the list sorted.

---- SYNCHRONIZATION ----

> > A4: How are race conditions avoided when multiple threads call
> > timer_sleep() simultaneously?

While adding a thread to the `sleeping_list`, interrupts are disabled using `intr_disable()` to prevent the timer interrupt handler from running in the middle of the operation. This ensures that the thread is added to the list atomically. The interrupts are enabled back to the previous state using `intr_set_level()` after the thread has been added to the list.

> > A5: How are race conditions avoided when a timer interrupt occurs
> > during a call to timer_sleep()?

Timer interrupts can not preempt the current thread in the middle of the `timer_sleep()` function. This is ensured by disabling interrupts using `intr_disable()` at the beginning of the function and enabling them back to the previous state using `intr_set_level()` at the end of the function. So, there can not be race conditions between the timer interrupt and the `timer_sleep()` function accessing the `sleeping_list`.

---- RATIONALE ----

> > A6: Why did you choose this design? In what ways is it superior to
> > another design you considered?

The current design using an ordered list for sleeping threads minimizes the time spent in the timer interrupt handler by ensuring that threads are woken up in the order of their wakeup time. While the overhead of inserting a thread into the ordered list is higher, it significantly reduces the time spent during the interrupt because only the threads that need to be woken up are checked. The alternative design of using an unordered list would require the timer interrupt to iterate through all sleeping threads on every tick, increasing the latency in the interrupt handler. Thus, the overhead of maintaining the ordered list is justified, as it reduces the overall interrupt latency.

Another design decision was to store the wakeup time for each thread as an absolute value (the number of ticks since the OS boot time) rather than storing the number of ticks the thread needs to sleep from the current time. This eliminates the need to decrement a variable for each sleeping thread on every tick. Instead, the system only needs to compare the current system ticks with the wakeup time of the first thread in the list, simplifying the check and reducing unnecessary work during each interrupt. This further optimizes the performance of the timer handler by minimizing the computations needed during each timer tick.

Another design decision was to disable interrupts while accessing the `sleeping_list` to avoid concurrent access by the timer interrupt handler and the `timer_sleep()` function. Using semaphore or locks would have been an alternative but would have added unnecessary complexity to the design since the timer interrupt handler and the `timer_sleep()` function are the only two places where the `sleeping_list` is accessed. If we used locks or semaphores, we would have to block the interrupt handler on the semaphore or lock acquire and release which comes with its own set of problems like transfering control to and from the interrupt handler in the middle of the operations. Thus, disabling interrupts was the best choice.
