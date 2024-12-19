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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SYS_gettaskinfo 441  // Define the syscall number for gettaskinfo

/**
 * main - Main function to demonstrate the usage of the `gettaskinfo` system call.
 * @argc: Argument count.
 * @argv: Argument vector.
 *
 * This program takes a PID as a command-line argument, invokes the `gettaskinfo` system call
 * to retrieve task information for the given PID, and prints the results.
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
    char buffer[256];  ///< Buffer to hold the information retrieved from the system call.
    long ret;          ///< Return value from the system call.

    // Make the system call to get task information.
    ret = syscall(SYS_gettaskinfo, pid, buffer);

    // Check if the system call was successful and handle errors.
    if (ret < 0) {
        fprintf(stderr, "syscall failed with error number %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }

    // Print the contents of the buffer after parsing it.
    long state;                 ///< Variable to hold the process state.
    unsigned long long start_time; ///< Variable to hold the process start time.
    int normal_prio;           ///< Variable to hold the process normal priority.

    // Parse the buffer to extract process information.
    sscanf(buffer, "%ld %llu %d", &state, &start_time, &normal_prio);

    // Output the parsed information to the standard output.
    printf("Process state: %ld\n", state);
    printf("Process start time: %llu\n", start_time);
    printf("Process normal priority: %d\n", normal_prio);

    return EXIT_SUCCESS;  // Indicate successful execution.
}
