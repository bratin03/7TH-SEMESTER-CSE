import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id = os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

sender_hash = "0x328Ff6652cc4E79f69B165fC570e3A0F468fc903"

# Query the latest block with full transaction details
payload_block = {
    "jsonrpc": "2.0",
    "method": "eth_getTransactionCount",
    "params": [ sender_hash, "finalized" ],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response_block = requests.post(url, headers=headers, data=json.dumps(payload_block))


if response_block.status_code == 200:
    result_block = response_block.json().get("result", {})
    count = int(result_block, 16)
    
    output = (
        f"Total transactions from {sender_hash}: {count}\n"
        f"JSON RPC Payload (Block): \n{json.dumps(payload_block, indent=None)}\n"
        f"JSON Response (Block): \n{json.dumps(response_block.json(), indent=None)}\n"
    )
    
    print(output)

# Write to file A5.txt
with open("A5.txt", "w") as file:
    file.write(output)
