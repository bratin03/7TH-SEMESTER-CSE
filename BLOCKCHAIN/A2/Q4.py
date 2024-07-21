import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Block hash to query
block_hash = "0x2625246aeea62714b5b4bea37a492081fcfb37e20d953014224e4bc9e21b5b2b"

# JSON-RPC payload to query the block by hash
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getBlockByHash",
    "params": [block_hash, True],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json().get("result", {})
    
    # Extract and print only the needed details
    total_difficulty = result.get("totalDifficulty", "N/A")
    block_number_hex = result.get("number", "0x0")
    block_number = int(block_number_hex, 16)
    parent_hash = result.get("parentHash", "N/A")
    transaction_root = result.get("transactionsRoot", "N/A")
    
    output = (
        f"I. Total Difficulty (in Hex): {total_difficulty}\n"
        f"II. Block Number (in Integer): {block_number}\n"
        f"III. Parent Hash: {parent_hash}\n"
        f"IV. Transaction Trie Root: {transaction_root}\n"
        f"V. JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"VI. JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file A4.txt
with open("A4.txt", "w") as file:
    file.write(output)
