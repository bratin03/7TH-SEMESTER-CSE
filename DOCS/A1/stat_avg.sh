#!/bin/bash

# Check if the user has provided an executable as input
if [ $# -ne 1 ]; then
    echo "Usage: $0 <executable>"
    exit 1
fi

EXECUTABLE=$1

# Check if the executable file exists and is executable
if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: The file '$EXECUTABLE' does not exist or is not executable."
    exit 1
fi

echo "Running perf stat on: $EXECUTABLE"

# Initialize variables for sums
total_branch_misses=0
total_page_faults=0
total_cache_misses=0
total_instructions=0
total_cycles=0

# Run the executable 10 times
for i in $(seq 1 100); do
    echo "Run #$i"
    
    output=$(sudo perf stat -e branch-misses,page-faults,cache-misses,instructions,cycles "$EXECUTABLE" 2>&1)

    # Extract values using grep and awk
    branch_misses=$(echo "$output" | grep "branch-misses" | awk '{print $1}' | tr -d ',')
    page_faults=$(echo "$output" | grep "page-faults" | awk '{print $1}' | tr -d ',')
    cache_misses=$(echo "$output" | grep "cache-misses" | awk '{print $1}' | tr -d ',')
    instructions=$(echo "$output" | grep "instructions" | awk '{print $1}' | tr -d ',')
    cycles=$(echo "$output" | grep "cycles" | awk '{print $1}' | tr -d ',')

    # Convert to numeric values
    branch_misses_val=$(echo $branch_misses | awk '{print ($1+0)}')
    page_faults_val=$(echo $page_faults | awk '{print ($1+0)}')
    cache_misses_val=$(echo $cache_misses | awk '{print ($1+0)}')
    instructions_val=$(echo $instructions | awk '{print ($1+0)}')
    cycles_val=$(echo $cycles | awk '{print ($1+0)}')

    if [ "$instructions_val" -ne 0 ] && [ "$cycles_val" -ne 0 ]; then
        ipc=$(echo "scale=2; $instructions_val / $cycles_val" | bc)
        echo "  Branch Misses: $branch_misses"
        echo "  Page Faults: $page_faults"
        echo "  Cache Misses: $cache_misses"
        echo "  Instructions per Cycle (IPC): $ipc"
    else
        echo "Error: Some performance metrics are missing or zero."
    fi

    # Accumulate totals
    total_branch_misses=$(echo "$total_branch_misses + $branch_misses_val" | bc)
    total_page_faults=$(echo "$total_page_faults + $page_faults_val" | bc)
    total_cache_misses=$(echo "$total_cache_misses + $cache_misses_val" | bc)
    total_instructions=$(echo "$total_instructions + $instructions_val" | bc)
    total_cycles=$(echo "$total_cycles + $cycles_val" | bc)

    echo ""
done

# Calculate averages
avg_branch_misses=$(echo "scale=2; $total_branch_misses / 100" | bc)
avg_page_faults=$(echo "scale=2; $total_page_faults / 100" | bc)
avg_cache_misses=$(echo "scale=2; $total_cache_misses / 100" | bc)
avg_instructions=$(echo "scale=2; $total_instructions / 100" | bc)
avg_cycles=$(echo "scale=2; $total_cycles / 100" | bc)

# Ensure that avg_instructions and avg_cycles are valid numbers before dividing
if [ "$(echo "$avg_instructions > 0" | bc)" -eq 1 ] && [ "$(echo "$avg_cycles > 0" | bc)" -eq 1 ]; then
    avg_ipc=$(echo "scale=2; $avg_instructions / $avg_cycles" | bc)
else
    avg_ipc="N/A"
fi

# Print summary
echo "Average Performance Metrics (over 100 runs):"
echo "  Average Branch Misses: $avg_branch_misses"
echo "  Average Page Faults: $avg_page_faults"
echo "  Average Cache Misses: $avg_cache_misses"
echo "  Average Instructions per Cycle (IPC): $avg_ipc"
