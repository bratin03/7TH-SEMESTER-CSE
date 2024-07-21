import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Block hash to query (in hex format with '0x' prefix)
block_hash = "0x4054F9"

# JSON-RPC payload to query the block by number
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getBlockTransactionCountByNumber",
    "params": [block_hash], # True to include full transactions
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json().get("result", "N/A")
    
    output = (
        f"Total transactions in block {block_hash}: {int(result, 16)}\n"
        f"JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )

# Print to stdout
print(output)

# Write to file A9.txt
with open("A9.txt", "w") as file:
    file.write(output)
