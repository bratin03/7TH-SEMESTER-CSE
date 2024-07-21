#!/bin/bash

# Check if the number of arguments is correct
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_runs>"
    exit 1
fi

num_runs=$1
exit_status=0

# Run the specified number of instances in parallel
for (( i=0; i<num_runs; i++ ))
do
    ./test_1.out $i &
done

# Wait for all background processes to finish
for job in `jobs -p`
do
    wait $job || exit_status=1
done

# Check the exit status and print the result
if [ $exit_status -eq 1 ]; then
    echo "One or more tests failed."
    echo "Failed"
else
    echo "All tests passed."
    echo "Passed"
fi

exit $exit_status
