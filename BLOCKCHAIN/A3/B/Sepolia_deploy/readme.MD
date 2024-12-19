# Steps to deploy smart contract in sepolia

1. Create a new directory and navigate into it:
```bash
mkdir Sepolia_deploy
cd Sepolia_deploy
```

2. Initialize a new Hardhat project
```bash
npm init -y
npm install --save-dev hardhat
npx hardhat
```

3. Install Dependencies
```bash
npm install --save-dev @nomiclabs/hardhat-ethers ethers
```

4. Put the smart contract in contracts folder. Currently, the smart contract is already present in contracts folder with name `BlockCart.sol`

5. Configure Hardhat for Sepolia. Currently its there in hardhat.config.js file. Use .env for YOUR_INFURA_PROJECT_ID and YOUR_PRIVATE_KEY

6. Compile smart contract
```bash
npx hardhat compile
```
7. Deploy the contract to Sepolia
```bash
npx hardhat run scripts/deploy.js --network sepolia
```
After running the above commands, you should see the contract address in the console output.
