/*
    * Author: Bratin Mondal
    * Roll No: 21CS10016

    * Deparment of Computer Science and Engineering
    * Indian Institue of Technology, Kharagpur
*/

#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H

#include <round.h>
#include <stdint.h>
/* ------------------------------ START OF CODE ADDED ------------------------------ */
#include <list.h> // Added to use list data structure
/* ------------------------------- END OF CODE ADDED ------------------------------- */

/* Number of timer interrupts per second. */
#define TIMER_FREQ 100

/*----------------------------- START OF CODE ADDED -----------------------------*/
int64_t next_wake; // The next wake up time for the wakeup thread
struct list sleepers; // List of sleeping threads
#define MAX_WAKEUP_COUNT 16 
// Maximum number of threads that can be woken up in one iteration holding the sleepers_lock
/*------------------------------ END OF CODE ADDED ------------------------------*/

void timer_init (void);
void timer_calibrate (void);

int64_t timer_ticks (void);
int64_t timer_elapsed (int64_t);
/* ------------------------------ START OF CODE ADDED ------------------------------ */
bool _timer_wakeup(int16_t max_wakeup_count); // Helper function to wake up sleeping threads by the wakeup thread upto max_wakeup_count threads in one iteration
void timer_wakeup(void); // Function to wake up sleeping threads by the wakeup thread
/* ------------------------------- END OF CODE ADDED ------------------------------- */

/* Sleep and yield the CPU to other threads. */
void timer_sleep (int64_t ticks);
void timer_msleep (int64_t milliseconds);
void timer_usleep (int64_t microseconds);
void timer_nsleep (int64_t nanoseconds);

/* Busy waits. */
void timer_mdelay (int64_t milliseconds);
void timer_udelay (int64_t microseconds);
void timer_ndelay (int64_t nanoseconds);

void timer_print_stats (void);

#endif /* devices/timer.h */
