/*
Testcase - 2: Opening the file twice
    1. A process can not open the file twice.

 * > 21CS30037 - Datta Ksheeraj
 * > 21CS10016 - Bratin Mondal
 *
 * Department of Computer Science and Engineering,
 * Indian Institute of Technology Kharagpur
 */

#include <set>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
using namespace std;

#define PROC_FILE "/proc/partb_21CS10016_21CS30037" // proc file to communicate with the module

/**
 * Main function to test the module
 * 
 * @return 0 on success
 */
int main()
{
    int fd = open(PROC_FILE, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int fd2 = open(PROC_FILE, O_RDONLY);
    if(fd2 < 0)
    {
        perror("Error opening file");
        printf("TEST PASSED\n");
        close(fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("TEST FAILED\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
}