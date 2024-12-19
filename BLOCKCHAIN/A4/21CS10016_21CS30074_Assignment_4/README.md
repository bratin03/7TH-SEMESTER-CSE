# Group Details
1. Bratin Mondal - 21CS10016
2. Abir Roy - 21CS30074

# Submission Details
Each directory contains the following structure:

1. appcode: Terminal application code to interact with the blockchain.
2. chaincode: Hyperledger Fabric smart contracts.
3. setup: Scripts to configure and start the blockchain (not included in the zip; refer to initial setup instructions).
4. run.sh: Script to launch the blockchain network.


# Setup Instructions
1. Download the fabric-samples repo with the fabric binaries.
2. Copy the `bin, config, and test-network` folders inside the `setup` folder of each part.
3. Copy `run.sh` script inside the `test-network` directory.


# Usage Instructions
To start the blockchain change directory to `<part>/setup/test-network` and run
```
chmod +x run.sh
./run.sh
```
Then change directory to `<part>/appcode/<org>` for each organisation and run
```
npm install
npm start
```
This will start the application for each organisation.

>> Note: After running the `run.sh` script, a new channel `mychannel` will be created. If the organisations already have a wallet that was used in some earlier channel, then the wallet will have to be reset. This can be done by deleting the `wallet` folder inside the `appcode/<org>` directory.

To reset the wallet, run the following command:
```
rm -rf wallet
```
