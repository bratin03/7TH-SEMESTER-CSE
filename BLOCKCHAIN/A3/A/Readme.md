# Running the code

1. Install the required packages using the following command:
```bash
npm install web3 dotenv

sudo npm install -g solc

sudo snap install solc
```

2. Run the following command to compile the contract:
```bash
solc --abi --bin --optimize --overwrite -o ./build NameRollRegistry.sol
```

3. Create a `.env` file in this directory and add the following:
```
INFURA_PROJECT_ID=<INFURA_PROJECT_ID>
PRIVATE_KEY=<PRIVATE_KEY>
ETHEREUM_ADDRESS=<ETHEREUM_ADDRESS>
CONTRACT_ADDRESS=0x61b0592cA07C420260f5D3d0d1ba7A0B7fDf7126
ABI_PATH=./build/NameRollRegistry.abi
```

4. Set your name and roll no in `script.js` and run the following command:
```bash
node script.js
```

