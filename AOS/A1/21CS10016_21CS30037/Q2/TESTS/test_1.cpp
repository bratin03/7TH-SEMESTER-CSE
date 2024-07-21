/*
Testcase - 1: Concurrent testing of the Module
    1. Randomly choose a size for the set from 1 to 100.
    2. Randomly choose values to insert into the set. (Both positive and negative values)
    3. With probability 0.8, check if the set is consistent after every insertion. (Dont check after every insertion simply to reduce overhead)
    4. Check at the end if the set is consistent.
    5. Use the test_1.sh script to run the test with multiple processes in parallel.

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
bool check(set<int> &s, int fd, int max_sz,int process_no)
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
 * Main function to test the module
 * @param argc The number of command line arguments
 * @param argv The command line arguments
 * 
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[])
{
    srand(getpid());

    if (argc == 2)
    {
        srand(atoi(argv[1]));
        printf("Process Number: %d\n", atoi(argv[1]));
    }
    else
    {
        printf("Usage: %s <process_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int process_no = atoi(argv[1]);

    set<int> s;
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

        // check if the set is consistent with probability 0.2
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
        printf("Process Number: %d, Test passed %d\n", process_no, ++count);
        close(fd);
    }
    printf("Process Number: %d, Test passed\n", process_no);
    exit(EXIT_SUCCESS);
}