require('dotenv').config();
const { Web3 } = require('web3');
const fs = require('fs');

// Load environment variables from .env file
const infuraProjectId = process.env.INFURA_PROJECT_ID;
const privateKey = process.env.PRIVATE_KEY;
const contractAddress = process.env.CONTRACT_ADDRESS;
// const myAddress = process.env.ETHEREUM_ADDRESS;
const abiPath = process.env.ABI_PATH;

// Load the compiled ABI file from Truffle or Hardhat build directory (update the path as necessary)
const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf-8'));

const web3 = new Web3(`https://sepolia.infura.io/v3/${infuraProjectId}`);

const contract = new web3.eth.Contract(contractABI, contractAddress);

// Wallet details
const account = web3.eth.accounts.privateKeyToAccount(privateKey);
web3.eth.accounts.wallet.add(account);
web3.eth.defaultAccount = account.address;

async function addProduct() {
    try {
        // Owner adds a product
        console.log('Updating a product...');

        const etherValue = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`current balance : ${  etherValue} ether`)
        
        const Receipt = await contract.methods.updateProduct(2, web3.utils.toWei('0.004', 'ether'))
            .send({ from: account.address, gas: 200000 });
        console.log('Product updated: ', Receipt);

        const etherValue1 = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`current balance : ${  etherValue1} ether`)

    } catch (error) {
        console.error('Error occurred:', error);
    }
}

addProduct();
