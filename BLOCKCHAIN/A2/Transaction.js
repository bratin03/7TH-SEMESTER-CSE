require('dotenv').config();
const { ethers, parseUnits } = require("ethers");

async function main() {
  try {
    // Log environment variables to verify they are loaded correctly
    console.log("ETHEREUM_NETWORK:", process.env.ETHEREUM_NETWORK);
    console.log("INFURA_API_KEY:", process.env.INFURA_API_KEY);
    console.log("SIGNER_PRIVATE_KEY:", process.env.SIGNER_PRIVATE_KEY);

    // Configuring the connection to an Ethereum node
    const network = process.env.ETHEREUM_NETWORK;
    const provider = new ethers.InfuraProvider(
      network,
      process.env.INFURA_API_KEY
    );

    console.log("Provider initialized:", provider);

    // Creating a signing account from a private key
    const signer = new ethers.Wallet(process.env.SIGNER_PRIVATE_KEY).connect(provider);
    console.log("Signer initialized:", signer.address);

    // Creating and sending the transaction object
    const tx = await signer.sendTransaction({
      to: "0xB80020a6561344BEEcfaB2CD53EBc6d60e08B3B4", // Replace with your selected account
      value: parseUnits("0.0002", "ether"),
    });

    console.log("Transaction sent, waiting for mining...");
    console.log(`Transaction Hash: ${tx.hash}`);
    console.log(`Check status at: https://${network}.etherscan.io/tx/${tx.hash}`);

    // Waiting for the transaction to be mined
    const receipt = await tx.wait();

    // The transaction is now on-chain!
    console.log(`Transaction mined in block ${receipt.blockNumber}`);
    console.log("Transaction receipt:", receipt);

  } catch (error) {
    console.error("Error occurred:", error);
  }
}

main();
