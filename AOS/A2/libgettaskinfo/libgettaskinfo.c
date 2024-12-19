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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include "libgettaskinfo.h"

// Define the system call number for `gettaskinfo`
#define __NR_gettaskinfo 441

// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // No-op
#endif

/**
 * gettaskinfo - Invokes the `gettaskinfo` system call to retrieve task information.
 * @pid: The process ID of the task to retrieve information for.
 *
 * This function makes a system call to retrieve task information for a given PID.
 * It parses the information into a `task_info` structure and returns a pointer to it.
 * If an error occurs, it returns NULL and sets errno accordingly.
 *
 * Returns:
 * * A pointer to a `task_info` structure on success.
 * * NULL on failure, with errno set to indicate the error.
 */
struct task_info* gettaskinfo(pid_t pid) {
    char buffer[256];           ///< Buffer to hold the raw information from the system call.
    struct task_info *info;     ///< Pointer to a structure to hold the parsed task information.
    long ret;                   ///< Return value from the system call.

    DEBUG_PRINT("Making syscall gettaskinfo with PID: %d\n", pid);

    // Make the system call to get task information.
    ret = syscall(__NR_gettaskinfo, pid, buffer);

    // Check for system call errors and print debug information if needed.
    if (ret < 0) {
        DEBUG_PRINT("System call failed with error number %d: %s\n", errno, strerror(errno));
        return NULL;
    }

    DEBUG_PRINT("System call succeeded. Buffer received: %s\n", buffer);

    // Allocate memory for the task_info structure.
    info = malloc(sizeof(struct task_info));
    if (!info) {
        DEBUG_PRINT("Memory allocation failed\n");
        errno = ENOMEM;  // Set errno to indicate memory allocation failure.
        return NULL;
    }

    // Parse the buffer to populate the task_info structure.
    char *state_str, *start_time_str, *normal_prio_str;

    // Tokenize the buffer based on spaces to extract task information.
    state_str = strtok(buffer, " ");
    start_time_str = strtok(NULL, " ");
    normal_prio_str = strtok(NULL, " ");

    // Check if all required tokens were extracted and convert them to appropriate types.
    if (state_str && start_time_str && normal_prio_str) {
        info->state = strtol(state_str, NULL, 10);       ///< Convert state from string to long.
        info->start_time = strtoull(start_time_str, NULL, 10);  ///< Convert start_time from string to unsigned long long.
        info->normal_prio = strtol(normal_prio_str, NULL, 10);   ///< Convert normal_prio from string to long.

        DEBUG_PRINT("Parsed info: state=%ld, start_time=%llu, normal_prio=%d\n",
                    info->state, info->start_time, info->normal_prio);
    } else {
        DEBUG_PRINT("Failed to parse buffer. Tokenized items: state_str=%s, start_time_str=%s, normal_prio_str=%s\n",
                    state_str, start_time_str, normal_prio_str);
        free(info);  // Free allocated memory if parsing fails.
        return NULL;
    }

    return info;  // Return the populated task_info structure.
}
