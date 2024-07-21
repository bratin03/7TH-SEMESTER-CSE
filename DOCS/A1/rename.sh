#!/bin/bash

# Check if a directory was provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

# Get the directory name from the argument
dir_path="$1"
dir_name=$(basename "$dir_path")

# Loop through all files in the specified directory
for file in "$dir_path"/*; do
    # Check if it's a regular file (not a directory)
    if [ -f "$file" ]; then
        # Rename the file by prefixing it with the directory name
        mv "$file" "$dir_path/${dir_name}_$(basename "$file")"
    fi
done
