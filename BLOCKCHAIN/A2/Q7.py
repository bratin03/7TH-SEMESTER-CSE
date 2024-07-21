import requests
import json
import os
from dotenv import load_dotenv

# Load the environment variables
load_dotenv()

# Replace with your Infura Project ID
infura_project_id =  os.environ.get("INFURA_API_KEY")
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# JSON-RPC payload to query the number of peers
payload = {
    "jsonrpc": "2.0",
    "method": "net_peerCount",
    "params": [],
    "id": 1
}

# Send the POST request
headers = {"Content-Type": "application/json"}
response = requests.post(url, headers=headers, data=json.dumps(payload))

# Prepare output content
if response.status_code == 200:
    result = response.json().get("result", "0x0")
    
    # Convert hex result to integer
    peer_count = int(result, 16)
    
    output = (
        f"I. Number of Peers (in Integer): {peer_count}\n"
        f"II. JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"III. JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file Q7.txt
with open("A7.txt", "w") as file:
    file.write(output)
