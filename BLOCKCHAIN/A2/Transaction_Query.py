import requests
import json
from web3 import Web3

# Replace with your Infura Project ID
infura_project_id = "8acb964b494d46bdb504e1fcf2d97a02"
url = f"https://sepolia.infura.io/v3/{infura_project_id}"

# Replace with the transaction hash you want to query
transaction_hash = "0x93fd8ddd5e5f7103a1369d60715ca27d6113e66e3efe90c6a7383acda1fb0b2a"

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
    
    # Extract details from the response
    from_address = result.get("from", "N/A")
    to_address = result.get("to", "N/A")
    value_wei = int(result.get("value", "0x0"), 16)  # Value in wei
    gas_used = int(result.get("gas", "0x0"), 16)  # Gas limit
    gas_price_wei = int(result.get("gasPrice", "0x0"), 16)  # Gas price in wei
    tx_hash = result.get("hash", "N/A")

    # Convert Wei to Ether manually
    value_eth = value_wei / 1e18
    gas_price_eth = gas_price_wei / 1e18

    output = (
        f"I. Transaction Hash: {tx_hash}\n"
        f"II. From Address: {from_address}\n"
        f"III. To Address: {to_address}\n"
        f"IV. Value Transferred: {value_wei} Wei ({value_eth} Ether)\n"
        f"V. Gas Used: {gas_used}\n"
        f"VI. Gas Price: {gas_price_wei} Wei ({gas_price_eth} Ether)\n"
        f"VII. JSON RPC Payload:\n{json.dumps(payload, indent=None)}\n"
        f"VIII. JSON Response:\n{json.dumps(response.json(), indent=None)}\n"
    )
else:
    output = (
        f"Error: {response.status_code}\n"
        f"{response.text}\n"
    )

# Print to stdout
print(output)

# Write to file TransactionDetails.txt
with open("TransactionDetails.txt", "w") as file:
    file.write(output)
