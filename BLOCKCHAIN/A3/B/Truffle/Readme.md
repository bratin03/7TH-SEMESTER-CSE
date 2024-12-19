# Steps to run smart contract using truffle

1. Install truffle globally using npm
```bash
sudo npm install -g truffle
```

2. Install ganache-cli globally using npm
```bash
sudo npm install -g ganache-cli
```

3. Initialize truffle project
```bash
truffle init
```

4. Put the smart contract in contracts folder. Currently, the smart contract is already present in contracts folder with name `BlockCart.sol`

5. Compile the smart contract
```bash
truffle compile
```

6. Create a migration file in migrations folder. Currently, the migration file is already present in migrations folder with name `2_deploy_blockcart.js`

7. Deploy the smart contract. First start ganache-cli
```bash
ganache-cli --port 8545
```
For using other port, change the port in truffle-config.js file

Now deploy the smart contract
```bash
truffle migrate
```

8. Put the test file in test folder. Currently, the test file is already present in test folder with name `BlockCart.js` and run the test
```bash
truffle test
```