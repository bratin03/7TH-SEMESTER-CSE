/*
Testcase - 4: Concurrent testing of the set with parent and child processes
    - Two processes are created using fork
    - Both processes write random values to the set to their own file descriptors
    - Both processes read the values from the set and check if the values are consistent
    - The parent process waits for the child process to complete
    - The parent process exits with success if the child process exits with success
    - The parent process exits with failure if the child process exits with failure

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
#include <sys/wait.h>
#include <sys/types.h>
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
 * @param process_no The process number
 * 
 * @return true if the set is consistent, false otherwise
 */
bool check(set<int> &s, int fd, int max_sz, int process_no)
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
            printf("Process Number: %d, Mismatch at index %d, expected %d, got %d\n", process_no, i, *it, set_read[i]);
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
 *  Function to run the test
 * @param process_no The process number
 *
 * @return void
 */
void run_test(int process_no)
{
    srand(getpid());

    int fd = open(PROC_FILE, O_RDWR, 0666);
    if (fd == -1)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    set<int> s;
    int size = rand() % 100 + 1;

    char capacity = (char)size;

    if (write(fd, &capacity, 1) != 1)
    {
        perror("Error writing capacity");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int val;
    int count = 0;

    for (int i = 0; i < size; i++)
    {
        usleep(rand() % 1000);
        // with equal probability, generate a positive or negative number
        val = prob(0.5) ? rand() : -rand();

        if (write(fd, &val, sizeof(int)) != sizeof(int))
        {
            perror("Error writing value to set");
            close(fd);
            exit(EXIT_FAILURE);
        }
        s.insert(val);

        // check if the set is consistent with probability 0.8
        if (prob(0.8))
        {
            if (!check(s, fd, size, process_no))
            {
                printf("Process Number: %d, Test failed\n", process_no);
                close(fd);
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("Process Number: %d, Test passed %d\n", process_no, ++count);
            }
        }
    }

    // check at the end
    if (!check(s, fd, size, process_no))
    {
        printf("Process Number: %d, Test failed\n", process_no);
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Process Number: %d, Final Test passed %d\n", process_no, ++count);
    }
    printf("Process Number: %d, Test passed\n", process_no);
    close(fd);
    if (process_no == 0)
    {
        exit(EXIT_SUCCESS);
    }
}

/**
 * Main function to run the test
 * 
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int main()
{
    fork();
    if (fork() == 0)
    {
        run_test(0);
    }
    else
    {
        run_test(1);
        int status;
        wait(&status);
        if(status != 0)
        {
            exit(EXIT_FAILURE);
        }
    }
    printf("Test passed\n");
    exit(EXIT_SUCCESS);
}
