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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "libgettaskinfo.h"

/**
 * main - Main function to demonstrate the usage of the `gettaskinfo` wrapper function.
 * @argc: Argument count.
 * @argv: Argument vector.
 *
 * This program takes a PID as a command-line argument, uses the `gettaskinfo` function to
 * retrieve task information for the given PID, and prints the results. It also handles
 * errors and provides usage information if the arguments are incorrect.
 *
 * Returns:
 * * EXIT_SUCCESS on successful execution.
 * * EXIT_FAILURE if there are errors or incorrect usage.
 */
int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments is provided.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Convert the PID from string to integer.
    pid_t pid = atoi(argv[1]);

    // Call the gettaskinfo function to retrieve information about the task.
    struct task_info *info = gettaskinfo(pid);

    // Check if gettaskinfo returned NULL and handle errors.
    if (info == NULL) {
        // Print specific error information if gettaskinfo uses errno for error reporting.
        fprintf(stderr, "gettaskinfo failed with error code %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    // Print the retrieved task information.
    printf("Process state: %ld\n", info->state);
    printf("Process start time: %llu\n", info->start_time);
    printf("Process normal priority: %d\n", info->normal_prio);

    // Free the allocated memory for the task_info structure.
    free(info);

    return EXIT_SUCCESS;  // Indicate successful execution.
}
