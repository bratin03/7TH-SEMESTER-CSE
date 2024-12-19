/*
 * CS60038: Advances in Operating Systems Design
 * Assignment 2
 * Members:
 * > 21CS30037 - Datta Ksheeraj
 * > 21CS10016 - Bratin Mondal
 *
 * Department of Computer Science and Engineering,
 * Indian Institute of Technology Kharagpur
 */

#ifndef LIBGETTASKINFO_H
#define LIBGETTASKINFO_H

#include <sys/types.h>

/**
 * struct task_info - Structure to hold information about a task.
 * @state: The state of the process, represented as a long integer.
 * @start_time: The start time of the process, represented as an unsigned long long integer.
 * @normal_prio: The normal priority of the process, represented as an integer.
 *
 * This structure is used to store information retrieved from the `gettaskinfo` system call.
 */
struct task_info {
    long state;              ///< Process state as a long integer.
    unsigned long long start_time; ///< Process start time as an unsigned long long integer.
    int normal_prio;        ///< Process normal priority as an integer.
};

/**
 * gettaskinfo - Retrieves information about a task with the given PID.
 * @pid: The process ID of the task to retrieve information for.
 *
 * This function acts as a wrapper around the `gettaskinfo` system call. It requests task
 * information for the specified PID and returns it in a `task_info` structure.
 *
 * Returns:
 * * A pointer to a `task_info` structure containing the requested task information on success.
 * * NULL on failure, with errno set to indicate the error.
 */
struct task_info* gettaskinfo(pid_t pid);

#endif // LIBGETTASKINFO_H
