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

async function buyProduct() {
    try {
        // Define the product and quantity the buyer wants to purchase
        const productId = 2;  // Replace with the product ID
        const quantity = 2;  // Replace with the desired quantity

        console.log(`Buying ${quantity} units of product ID ${productId}...`);

        const etherValue = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`Current balance: ${etherValue} ether`);

        // Get the product price from the smart contract
        const product = await contract.methods.products(productId).call();
        const totalPrice = BigInt(product.price) * BigInt(quantity);
        console.log(`Total price for ${quantity} units: ${web3.utils.fromWei(totalPrice.toString(), 'ether')} ether`);

        // Send the transaction to buy the product
        const buyReceipt = await contract.methods.buyProduct(productId, quantity)
            .send({ 
                from: account.address, 
                value: totalPrice.toString(),  // Send the required amount of ether
                gas: 400000 
            });
        console.log('Product purchased successfully: ', buyReceipt);

        const etherValueAfter = web3.utils.fromWei(BigInt(await web3.eth.getBalance(account.address)), 'ether');
        console.log(`Balance after purchase: ${etherValueAfter} ether`);

    } catch (error) {
        console.error('Error occurred:', error);
    }
}

buyProduct();
