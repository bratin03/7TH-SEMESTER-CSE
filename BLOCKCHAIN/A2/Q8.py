import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Transaction hash to query
transaction_hash = "0xf3671a293383e03dd12639410716492c2cc600e7c7c2464aa432f24964cb585e"

# JSON-RPC payload to query the transaction receipt by hash
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getTransactionReceipt",
    "params": [transaction_hash],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json().get("result", {})
    
    # Extract details
    block_number_hex = result.get("blockNumber", "0x0")
    block_number = int(block_number_hex, 16)
    block_hash = result.get("blockHash", "0x0")
    cumulative_gas_used_hex = result.get("cumulativeGasUsed", "0x0")
    cumulative_gas_used = int(cumulative_gas_used_hex, 16)
    transaction_index_hex = result.get("transactionIndex", "0x0")
    transaction_index = int(transaction_index_hex, 16)
    
    output = (
        f"I. Block Number (in Integer): {block_number}\n"
        f"II. Block Hash: {block_hash}\n"
        f"III. Cumulative Gas Used (in Integer): {cumulative_gas_used}\n"
        f"IV. Transaction Index (in Integer): {transaction_index}\n"
        f"V. JSON RPC Request:\n{json.dumps(payload, indent=None)}\n"
        f"VI. JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file Q8.txt
with open("A8.txt", "w") as file:
    file.write(output)
