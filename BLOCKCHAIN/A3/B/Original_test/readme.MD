# Running the code

1. Install the required packages using the following command:
```bash
npm install web3 dotenv

npm install -g solc@0.8.19

```

2. Run the following command to compile the contract:
```bash
solcjs --bin --abi -o build/ BlockCart.sol
```

3. Create a `.env` file in this directory and add the following:
```
INFURA_PROJECT_ID=<INFURA_PROJECT_ID>
PRIVATE_KEY=<PRIVATE_KEY>
ETHEREUM_ADDRESS=<ETHEREUM_ADDRESS>
CONTRACT_ADDRESS=0xA6f32f64D72e9bB019F11A1160239f2F4668050A
ABI_PATH=./build/BlockCart_sol_BlockCart.abi
```

4. Set your name and roll no in `script.js` and run the following command:
```bash
node <yourscript>.js
```
