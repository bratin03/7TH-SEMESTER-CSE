/*
Testcase - 5: Initializing with invalid size
    1. Write 1 byte of invalid size (Not in the range of 1-100)
    2. Write 2 bytes of invalid size (Expected is 1 byte)
    3. Write 4 bytes of invalid size (Expected is 1 byte)

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

    // 1 byte of invalid size
    char val[] = {(char)-10,char(-1),char(0),char(101),char(127)};  
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

    // 2 bytes of invalid size
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

    // 4 bytes of invalid size
    unsigned int val4[] = {0, 1, 2, 3, 4};
    for(int i = 0; i < 5; i++)
    {
        int sz = write(fd, val4 + i, 4);
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