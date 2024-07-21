import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Replace with the Ethereum address you want to check
account_address = "0x5d388d58dae954c602fc3f03f8ccbeb64189c7d5"

# JSON-RPC payload to query the balance
payload = {
    "jsonrpc": "2.0",
    "method": "eth_getBalance",
    "params": [account_address, "latest"],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json().get("result", "0x0")
    # Convert hex result to integer (wei)
    balance_wei = int(result, 16)
    balance_eth = balance_wei / 10**18    
    
    output = (
        f"Account Balance (in Wei): {balance_wei}\n"
        f"JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
        f"Account Balance (in Ether): {balance_eth}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)
