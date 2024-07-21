#!/bin/bash

# Load environment variables from .env file
if [ -f .env ]; then
    while IFS='=' read -r key value; do
        if [[ ! -z "$key" && "$key" != \#* ]]; then
            key=$(echo $key | xargs) # Trim whitespace
            value=$(echo $value | sed 's/^"\(.*\)"$/\1/' | xargs) # Remove surrounding quotes and trim whitespace
            export "$key"="$value"
        fi
    done < .env
fi

# Replace with your Infura Project ID from the .env file
INFURA_PROJECT_ID="$INFURA_API_KEY"
URL="https://sepolia.infura.io/v3/${INFURA_PROJECT_ID}"

# Address to filter transactions (loaded from .env)
ADDRESS="$ADDRESS"

# Query the latest block with full transaction details
payload_block='{
    "jsonrpc": "2.0",
    "method": "eth_getBlockByNumber",
    "params": ["latest", true],
    "id": 1
}'

# Send the POST request
response_block=$(curl -s -w "%{http_code}" -X POST $URL -H "Content-Type: application/json" -d "$payload_block")
response_block_body=$(echo "$response_block" | sed '$d')  # Remove HTTP status code from response
status_code=$(echo "$response_block" | tail -n1)

# Check if response is valid
if [[ "$status_code" -ne 200 ]]; then
    echo "Error: $(echo "$response_block_body" | jq -r '.error.message')"
    exit 1
fi

# Extract the transactions from the response
transactions=$(echo $response_block_body | jq -r '.result.transactions[]')

# Count transactions sent from the specified address
transaction_count=0

for tx in $(echo $transactions | jq -r '.hash'); do
    # Extract the 'from' address from each transaction
    tx_from=$(echo $response_block_body | jq -r --arg tx "$tx" '.result.transactions[] | select(.hash==$tx) | .from')
    
    if [ "$tx_from" = "$ADDRESS" ]; then
        transaction_count=$((transaction_count + 1))
    fi
done

# Prepare output
output="I. Number of transactions sent from the address: $transaction_count\n"
output+="II. JSON RPC Payload to get latest block details:\n$payload_block\n"
output+="III. JSON Response:\n$response_block_body\n"

# Print to stdout
echo -e "$output"

# Write to file A5.txt
echo -e "$output" > A5.txt
