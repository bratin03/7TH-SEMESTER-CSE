/*
Testcase - 3: Resetting the set
    1. A process can reset the set by reopening the file.
    2. The number of times the set is reset is given as a command line argument.

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

#define PROC_FILE "/proc/partb_21CS10016_21CS30037" // proc file to communicate with the module

/**
 * Function to generate a random number between 0 and 1
 * @param p The probability of getting 1
 * 
 * @return 1 with probability p, 0 otherwise
 */
int prob(double p)
{
    return rand() < p * RAND_MAX;
}

/**
 * Function to check if the set is consistent with the values read from the file
 * @param s The set to check
 * @param fd The file descriptor to read the values from
 * @param max_sz The maximum size of the set
 * 
 * @return true if the set is consistent, false otherwise
 */
bool check(set<int> &s, int fd, int max_sz)
{
    int set_read[max_sz];
    int sz = read(fd, set_read, sizeof(set_read));
    if (sz < 0)
    {
        perror("Error reading values from set");
        close(fd);
        exit(EXIT_FAILURE);
    }
    auto it = s.begin();
    for (int i = 0; i < sz / sizeof(int); i++)
    {
        if (*it != set_read[i])
        {
            printf("Mismatch at index %d, expected %d, got %d\n", i, *it, set_read[i]);
            return false;
        }
        it++;
    }
    if (it != s.end())
    {
        return false;
    }
    return true;
}

/**
 * Main function to test the module
 * 
 * @return 0 on success
 */
int main(int argc,char *argv[])
{
    int iterations =0;
    if(argc == 2)
    {
        iterations = atoi(argv[1]);
    }
    else
    {
        printf("Usage: %s <iterations>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    set<int> s;
    srand(time(0));
    for(int i=0;i<=iterations;i++)
    {
        int fd = open(PROC_FILE, O_RDWR);
        if (fd == -1)
        {
            perror("Error opening proc file");
            exit(EXIT_FAILURE);
        }

        int size = rand() % 100 + 1;

        char capacity = (char)size;
        if (write(fd, &capacity, 1) != 1)
        {
            perror("Error writing capacity");
            close(fd);
            exit(EXIT_FAILURE);
        }

        s.clear();
        for (int i = 0; i < size; i++)
        {
            int num = (prob(0.5) ? rand() : -rand());
            s.insert(num);
            if (write(fd, &num, sizeof(num)) != sizeof(num))
            {
                perror("Error writing to set");
                close(fd);
                exit(EXIT_FAILURE);
            }
        }

        if (!check(s, fd, size))
        {
            printf("TEST FAILED at iteration %d\n",i);
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("TEST PASSED at iteration %d\n",i);
        }
        close(fd);
    }
    printf("TEST PASSED\n");
    exit(EXIT_SUCCESS);
}