/*
Testcase - 9: Child process tries to write to parent file descriptor
    - The child process should not be able to write to the parent file descriptor

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
#include <sys/wait.h>
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

    if(fork() == 0)
    {
        int x = rand();
        int ret = write(fd, &x, sizeof(int));
        if(ret<0)
        {
            perror("write");
            printf("Child process was not able to write to parent file descriptor\n");
            printf("TEST PASSED\n");
        }
        else if(ret==sizeof(int))
        {
            printf("Child process was able to write to parent file descriptor\n");
            printf("TEST FAILED\n");
        }
        else
        {
            printf("Something went wrong\n");
            printf("TEST FAILED\n");
        }
    }
    else
    {
        wait(nullptr);
    }

    close(fd);
    return 0;
}