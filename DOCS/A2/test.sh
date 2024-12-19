#!/bin/bash

# Initialize counters for each test
single_pass=0
single_fail=0
multiple_pass=0
multiple_fail=0
simultaneous_pass=0
simultaneous_fail=0

# Function to run a single test and track pass/fail
run_test() {
    local test_name=$1
    local output_file="tests/threads/$test_name.result"

    # Run the test
    pintos -v -k -T 60 --qemu -- -q run $test_name < /dev/null 2> tests/threads/$test_name.errors > tests/threads/$test_name.output
    perl -I../.. ../../tests/threads/$test_name.ck tests/threads/$test_name tests/threads/$test_name.result

    # Check if the test passed or failed
    if grep -q "PASS" "$output_file"; then
        echo -e "\e[32m$test_name passed\e[0m"  # Green color for pass
        if [ "$test_name" == "alarm-single" ]; then
            single_pass=$((single_pass + 1))
        elif [ "$test_name" == "alarm-multiple" ]; then
            multiple_pass=$((multiple_pass + 1))
        elif [ "$test_name" == "alarm-simultaneous" ]; then
            simultaneous_pass=$((simultaneous_pass + 1))
        fi
    else
        echo -e "\e[31m$test_name failed\e[0m"  # Red color for fail
        if [ "$test_name" == "alarm-single" ]; then
            single_fail=$((single_fail + 1))
        elif [ "$test_name" == "alarm-multiple" ]; then
            multiple_fail=$((multiple_fail + 1))
        elif [ "$test_name" == "alarm-simultaneous" ]; then
            simultaneous_fail=$((simultaneous_fail + 1))
        fi
    fi
}

# Ensure the script takes one argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_runs>"
    exit 1
fi

# Read the number of runs from the first argument
num_runs=$1

# Loop for the specified number of runs
for ((i = 1; i <= num_runs; i++)); do
    echo "Run #$i"
    run_test "alarm-single"
    run_test "alarm-multiple"
    run_test "alarm-simultaneous"
    echo "--------------------------------"
done

# Print the total statistics at the end
echo "===== Summary ====="
echo -e "\e[32malarm-single passed $single_pass times\e[0m"
echo -e "\e[31malarm-single failed $single_fail times\e[0m"
echo -e "\e[32malarm-multiple passed $multiple_pass times\e[0m"
echo -e "\e[31malarm-multiple failed $multiple_fail times\e[0m"
echo -e "\e[32malarm-simultaneous passed $simultaneous_pass times\e[0m"
echo -e "\e[31malarm-simultaneous failed $simultaneous_fail times\e[0m"
