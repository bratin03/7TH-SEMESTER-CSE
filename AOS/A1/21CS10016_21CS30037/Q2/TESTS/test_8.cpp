/*
Testcase - 8: Try to write more than the size of the set
    1. Choose a random size for the set
    2. Write more than the size of the set 10 times // These extra writes should fail

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
#include <time.h>
using namespace std;

#define PROC_FILE "/proc/partb_21CS10016_21CS30037"

/**
 * Main function to test the module
 * 
 * @return 0 on success
 */
int main()
{
    srand(time(0));
    int sz = rand() % 100 + 1;
    int fd = open(PROC_FILE, O_RDWR, 0666);
    if(fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char size=(char)sz;
    int ret=write(fd,&size,sizeof(char));
    if(ret<0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < sz + 10; i++)
    {
        int x = i;
        int ret = write(fd, &x, sizeof(int));
        if(i>=sz)
        {
            if(ret<0)
            {
                printf("TEST CASE %d PASSED\n", i + 1);
                printf("Attempting to write more than the size of the set\n");
            }
            else
            {
                printf("TEST CASE %d FAILED\n", i + 1);
            }
        }
        else
        {
            if(ret==sizeof(int))
            {
                printf("TEST CASE %d PASSED\n", i + 1);
            }
            else
            {
                printf("TEST CASE %d FAILED\n", i + 1);
            }
        }
    }

    close(fd);
    return 0;
}