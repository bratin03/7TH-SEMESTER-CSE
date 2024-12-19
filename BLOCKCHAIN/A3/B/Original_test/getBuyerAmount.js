require('dotenv').config();
const { Web3 } = require('web3');
const fs = require('fs');

// Load environment variables from .env file
const infuraProjectId = process.env.INFURA_PROJECT_ID;
const contractAddress = process.env.CONTRACT_ADDRESS;
const abiPath = process.env.ABI_PATH;
const buyerAddress = process.env.BUYER_ADDRESS; // Address of the buyer to query

// Load the compiled ABI file from Truffle or Hardhat build directory
const contractABI = JSON.parse(fs.readFileSync(abiPath, 'utf-8'));

const web3 = new Web3(`https://sepolia.infura.io/v3/${infuraProjectId}`);

const contract = new web3.eth.Contract(contractABI, contractAddress);

async function getBuyerAmountPaid() {
    try {
        console.log(`Fetching total amount paid by buyer: ${buyerAddress}`);

        // Call the smart contract function
        const amountPaid = await contract.methods.getBuyerAmountPaid(buyerAddress).call();
        
        // Convert the amount from wei to ether for better readability
        const amountPaidInEther = web3.utils.fromWei(amountPaid, 'ether');
        
        console.log(`Total amount paid by buyer ${buyerAddress}: ${amountPaidInEther} ether`);
    } catch (error) {
        console.error('Error occurred:', error);
    }
}

getBuyerAmountPaid();
