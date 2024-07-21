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

# Run perf stat to collect performance data and format the output
sudo perf stat -e branch-misses,page-faults,cache-misses,instructions,cycles "$EXECUTABLE" 2>&1 | \
awk '
    /branch-misses/ { branch_misses=$1 }
    /page-faults/ { page_faults=$1 }
    /cache-misses/ { cache_misses=$1 }
    /instructions/ { instructions=$1 }
    /cycles/ { cycles=$1 }
    END {
        # Remove commas if present
        gsub(/,/, "", branch_misses)
        gsub(/,/, "", page_faults)
        gsub(/,/, "", cache_misses)
        gsub(/,/, "", instructions)
        gsub(/,/, "", cycles)

        branch_misses_val = strtonum(branch_misses)
        page_faults_val = strtonum(page_faults)
        cache_misses_val = strtonum(cache_misses)
        instructions_val = strtonum(instructions)
        cycles_val = strtonum(cycles)

        if (instructions_val && cycles_val) {
            ipc = instructions_val / cycles_val
            printf "Branch Misses: %s\n", branch_misses
            printf "Page Faults: %s\n", page_faults
            printf "Cache Misses: %s\n", cache_misses
            printf "Instructions per Cycle (IPC): %.2f\n", ipc
        } else {
            print "Error: Some performance metrics are missing or zero."
        }
    }
'

# Check if the perf command was successful
if [ $? -ne 0 ]; then
    echo "Error: perf stat command failed."
    exit 1
fi
