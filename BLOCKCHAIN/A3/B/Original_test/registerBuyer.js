require('dotenv').config();
const { Web3 } = require('web3');
const fs = require('fs');

// Load environment variables from .env file
const infuraProjectId = process.env.INFURA_PROJECT_ID;
const privateKey = process.env.PRIVATE_KEY2;
const contractAddress = process.env.CONTRACT_ADDRESS;
// const myAddress = process.env.ETHEREUM_ADDRESS;
const abiPath = process.env.ABI_PATH;

// Load the compiled ABI file from Truffle or Hardhat build directory
const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf-8'));

const web3 = new Web3(`https://sepolia.infura.io/v3/${infuraProjectId}`);

const contract = new web3.eth.Contract(contractABI, contractAddress);

// Wallet details
const account = web3.eth.accounts.privateKeyToAccount(privateKey);
web3.eth.accounts.wallet.add(account);
web3.eth.defaultAccount = account.address;

async function registerBuyer() {
    try {
        // Register a buyer
        console.log('Registering a buyer...');

        const buyerName = "Bratin Mondal";  // Replace with buyer's name
        const buyerEmail = "BM@gmail.com";  // Replace with buyer's email
        const buyerMailingAddress = "IIT KGP";  // Replace with mailing address

        const etherValue = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`Current balance: ${etherValue} ether`);

        const registerReceipt = await contract.methods.registerBuyer(buyerName, buyerEmail, buyerMailingAddress)
            .send({ from: account.address, gas: 400000 });
        console.log('Buyer registered: ', registerReceipt);

        const etherValueAfter = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`Current balance after registration: ${etherValueAfter} ether`);

    } catch (error) {
        console.error('Error occurred:', error);
    }
}

registerBuyer();
