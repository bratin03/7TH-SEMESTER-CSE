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
transaction_hash = "0x9c155c9b20480e483c40573edca0bc7c0ffc19fc5bf05d859fbb0f0cd47799c7"

# JSON-RPC payload to query the transaction by hash
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getTransactionByHash",
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
    value_hex = result.get("value", "0x0")
    value_wei = int(value_hex, 16)
    gas_used = result.get("gas", "0x0")
    gas_used_integer = int(gas_used, 16)
    
    output = (
        f"I. Value Transferred (in Wei, Integer): {value_wei}\n"
        f"II. Gas Used (in Integer): {gas_used_integer}\n"
        f"III. JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"IV. JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file Q6.txt
with open("A6.txt", "w") as file:
    file.write(output)
