require('dotenv').config();
const { Web3 } = require('web3');
const fs = require('fs');

// Load environment variables from .env file
const infuraProjectId = process.env.INFURA_PROJECT_ID;
const privateKey = process.env.PRIVATE_KEY;
const contractAddress = process.env.CONTRACT_ADDRESS;
const myAddress = process.env.ETHEREUM_ADDRESS;
const abiPath = process.env.ABI_PATH;


// Initialize web3 with Infura provider
const web3 = new Web3(`https://sepolia.infura.io/v3/${infuraProjectId}`);

// Add the account to web3
const account = web3.eth.accounts.privateKeyToAccount(privateKey);
web3.eth.accounts.wallet.add(account);
web3.eth.defaultAccount = account.address;

// Read ABI from the file
let abi;
try {
    abi = JSON.parse(fs.readFileSync(abiPath, 'utf8'));
} catch (error) {
    console.error('Failed to read ABI:', error);
    process.exit(1);
}

// Initialize the contract with the ABI and address
const contract = new web3.eth.Contract(abi, contractAddress);

// Function to query the name and roll number for a specific address
async function queryNameAndRoll(address) {
    try {
        const result = await contract.methods.get(address).call();
        console.log(`Name: ${result[0]}, Roll: ${result[1]}`);
    } catch (error) {
        console.error('Error querying name and roll:', error);
    }
}

async function queryNameAndRollMine() {
    try {
        const result = await contract.methods.getmine().call();
        console.log(`Name: ${result[0]}, Roll: ${result[1]}`);
    } catch (error) {
        console.error('Error querying name and roll:', error);
    }
}

// Function to update your own name and roll number
async function updateNameAndRoll(name, roll) {
    try {
        const tx = await contract.methods.update(name, roll).send({ from: myAddress, gas: 302084 });
        console.log('Transaction ID:', tx.transactionHash);
    } catch (error) {
        console.error('Error updating name and roll:', error);
    }
}



// Example usage:

// Task B.1: Query name and roll for a specific address
queryNameAndRoll('0x328Ff6652cc4E79f69B165fC570e3A0F468fc903');

// Task B.2: Update your own name and roll number
// updateNameAndRoll('Bratin Mondal','21CS10016')
// updateNameAndRoll('Abir Roy','21CS30074')

// Testing 
// Check name and roll for your own address
queryNameAndRoll(myAddress);
// queryNameAndRollMine();

// Txhash : 0xce7f2967a02ce5ab586f0d53cc763adf474190421f3c8db1ed2a13a72bbe8e65
