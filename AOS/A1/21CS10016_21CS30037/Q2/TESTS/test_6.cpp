/*
Testcase - 6: Write invalid data // Expected size is 4 bytes
    1. Write 1 byte of invalid data
    2. Write 2 bytes of invalid data
    3. Write 8 bytes of invalid data

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

#define PROC_FILE "/proc/partb_21CS10016_21CS30037"

/**
 * Main function to test the module
 * 
 * @return 0 on success
 */
int main()
{
    int fd = open(PROC_FILE, O_RDWR, 0666);

    // Initialize with 16
    char x=16;
    int ret=write(fd,&x,sizeof(char));

    // 1 byte of invalid data
    char val[] = {(char)0, (char)1, (char)2, (char)3, (char)4};
    for(int i = 0; i < 5; i++)
    {
        int sz = write(fd, val + i, 1);
        if (sz < 0)
        {
            printf("TEST CASE %d PASSED\n", i + 1);
        }
        else
        {
            printf("TEST CASE %d FAILED\n", i + 1);
        }
    }

    // 2 bytes of invalid data
    unsigned short val2[] = {0, 1, 2, 3, 4};
    for(int i = 0; i < 5; i++)
    {
        int sz = write(fd, val2 + i, 2);
        if (sz < 0)
        {
            printf("TEST CASE %d PASSED\n", i + 6);
        }
        else
        {
            printf("TEST CASE %d FAILED\n", i + 6);
        }
    }

    // 8 bytes of invalid data
    unsigned long long val8[] = {0, 1, 2, 3, 4};
    for(int i = 0; i < 5; i++)
    {
        int sz = write(fd, val8 + i, 8);
        if (sz < 0)
        {
            printf("TEST CASE %d PASSED\n", i + 11);
        }
        else
        {
            printf("TEST CASE %d FAILED\n", i + 11);
        }
    }

    close(fd);
    return 0;
}