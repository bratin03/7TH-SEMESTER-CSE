import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Ethereum address to query
address = "0x328Ff6652cc4E79f69B165fC570e3A0F468fc903"

# JSON-RPC payload to query the balance
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getBalance",
    "params": [address, "latest"],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json()["result"]
    # Convert hex result to integer (wei)
    balance_wei = int(result, 16)
    
    output = (
        f"Answer: {balance_wei}\n"
        f"JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file A3.txt
with open("A3.txt", "w") as file:
    file.write(output)
