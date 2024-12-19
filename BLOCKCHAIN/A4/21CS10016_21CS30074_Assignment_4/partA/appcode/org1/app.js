/**
 * Student Information
 * 
 * Bratin Mondal - 21CS10016
 * Abir Roy - 21CS30074
 * 
 * Department of Computer Science and Engineering
 * Indian Institute of Technology, Kharagpur
 */

"use strict";

const { Gateway, Wallets } = require("fabric-network");
const FabricCAServices = require("fabric-ca-client");
const path = require("path");
const readline = require("readline");
const {
  buildCAClient,
  registerAndEnrollUser,
  enrollAdmin,
} = require("../CAUtil.js");
const { buildCCPOrg1, buildWallet } = require("../AppUtil.js");

// Create a readline interface
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

// Constants
const myChannel = "mychannel";
const myChaincodeName = "mychaincode";
const org1PrivateCollectionName = "Org1MSPPrivateCollection";
const mspOrg1 = "Org1MSP";
const Org1UserId = "appUser1";
var identity = null;

/**
 * Function to ask a question to the user
 * 
 * @param {string} question 
 * @returns {Promise<string>} The user's answer
 */
function askQuestion(question) {
  return new Promise((resolve) => {
    rl.question(question, (answer) => {
      resolve(answer);
    });
  });
}

/**
 * Function to print the details of a document
 * 
 * @param {Object} doc The document object
 * @returns {void}
 */
function printDocDetails(doc) {
  console.log(`Document Title: ${doc.docTitle}`);
  console.log(`Document Data: ${doc.docData}`);
  console.log(`Document Price: ${doc.price}`);
  console.log(`Document Hash: ${doc.docHash}`);
}

/**
 * Function to print the list of documents
 * 
 * @param {Array<Object>} docs The list of documents
 * @returns {void}
 */
function printDocList(docs) {
  docs.forEach((doc) => {
    console.log(`Document ID: ${doc.docID}`);
    printDocDetails(doc);
    console.log("\n");
  });
}

const RED = "\x1b[31m";
const GREEN = "\x1b[32m";
const RESET = "\x1b[0m";

/**
 * Function to initialize the contract using the Org1 identity
 * 
 * @returns {Promise<Gateway>} The gateway object
 * @throws {Error} If an error occurs
 */
async function initContractFromOrg1Identity() {
  console.log(
    `${GREEN}--> Fabric client user & Gateway init: Using Org1 identity to Org1 Peer${RESET}`
  );
  const ccpOrg1 = buildCCPOrg1();
  const caOrg1Client = buildCAClient(
    FabricCAServices,
    ccpOrg1,
    "ca.org1.example.com"
  );
  const walletPathOrg1 = path.join(__dirname, "wallet/org1");
  const walletOrg1 = await buildWallet(Wallets, walletPathOrg1);

  identity = await walletOrg1.get(Org1UserId);

  // Enroll admin and register user
  await enrollAdmin(caOrg1Client, walletOrg1, mspOrg1);
  await registerAndEnrollUser(caOrg1Client, walletOrg1, mspOrg1, Org1UserId, "org1.department1");

  try {
    const gatewayOrg1 = new Gateway();
    await gatewayOrg1.connect(ccpOrg1, {
      wallet: walletOrg1,
      identity: Org1UserId,
      discovery: { enabled: true, asLocalhost: true },
    });
    return gatewayOrg1;
  } catch (error) {
    console.error(`${RED}Error in connecting gateway using Org1 identity: ${error}${RESET}`);
    process.exit(1);
  }
}

// Main function for the terminal-based application
async function main() {
  let gatewayOrg1;
  try {
    gatewayOrg1 = await initContractFromOrg1Identity();
    const networkOrg1 = await gatewayOrg1.getNetwork(myChannel);
    const contractOrg1 = networkOrg1.getContract(myChaincodeName);

    // Add discovery interest
    contractOrg1.addDiscoveryInterest({
      name: myChaincodeName,
      collectionNames: [org1PrivateCollectionName],
    });
    if (identity == null) {
      // Initialize the ledger
      await contractOrg1.submitTransaction("InitLedger");
    }
    // Command loop
    while (true) {

      // Ask the user for a command
      const command = await askQuestion(`
Enter a command:
1. ADD_MONEY <amount> - Adds the specified amount to your balance
2. ADD_DOC <docID> <docTitle> <docData> <price> - Adds a new document with the specified details
3. QUERY_BALANCE - Retrieves the current balance
4. UPDATE_DOC_DATA <docID> <newDocData> <updateHash> - Updates the document data with the new information
5. GET_ALL_DOCS - Retrieves all stored documents
6. GET_DOC <docID> - Retrieves the document with the specified ID
7. EXIT - Exits the application

Please enter your command:
`);

      if (command === "EXIT") {
        console.log("Exiting the application...");
        break;
      }

      try {
        await handleCommand(command, contractOrg1); // Call the command handler
      } catch (error) {
        console.error(`Error in transaction:`);
        // Print the error message in RED color
        console.error(`${RED}${error}${RESET}`);
      }
    }
  } catch (error) {
    console.error(`Error: ${error}`);
    console.error(error.stack ? error.stack : "");
    process.exit(1);
  } finally {
    // Ensure to disconnect gateway and close readline
    rl.close();
    try {
      await gatewayOrg1.disconnect();
    }
    catch (error) {
      console.error(`Error in disconnecting gateway: ${error}`);
    }
  }
}


/**
 * Function to handle the user command
 * 
 * @param {string} command The user command
 * @param {Contract} contractOrg The contract object for Org
 * @returns {Promise<void>}
 * @throws {Error} If an error occurs
 */
async function handleCommand(command, contractOrg) {
  if (command.startsWith("ADD_MONEY")) {
    /**
     * ADD_MONEY <amount>
     *
     * Add specified amount to the balance of the user
     */
    // Check if the amount is provided
    if (!command.split(" ")[1]) {
      console.error("Please provide the amount to add.\n");
      return;
    }

    const amount = parseInt(command.split(" ")[1]);

    // Validate the amount
    if (isNaN(amount)) {
      console.error("Invalid amount. Please provide a valid number.\n");
      return;
    }

    // Ensure the amount is non-negative
    if (amount < 0) {
      console.error("Invalid amount: must be non-negative.\n");
      return;
    }

    const tmapData = Buffer.from(JSON.stringify({ amount }));

    let statefulTxn = contractOrg.createTransaction("AddBalance");
    statefulTxn.setTransient({
      amount: tmapData, // Set the key here to match the chaincode
    });

    try {
      const response = await statefulTxn.submit();
      // Print success message and the updated balance
      console.log(
        `Added ${amount} to the balance. New balance is ${response.toString()}\n`
      );
    } catch (error) {
      console.error("Error adding money to the balance:\n");
      throw error;
    }
  } else if (command.startsWith("ADD_DOC")) {
    /**
     * ADD_DOC <docID> <docTitle> <docData> <price>
     *
     * Add a new document to the ledger
     * docID: Unique identifier for the document
     * docTitle: Title of the document
     * docData: Data of the document
     * price: Price of the document (must be a positive number)
     */

    const [_, docID, docTitle, docData, price] = command.split(" ");

    // Check if the parameters are provided
    if (!docID || !docTitle || !docData || !price) {
      console.error("Please provide all the required parameters.\n");
      return;
    }

    // Validate input parameters
    if (isNaN(parseInt(price)) || parseInt(price) < 0) {
      console.error("Invalid price. Please provide a positive number.\n");
      return;
    }

    // Prepare transient data for the transaction
    const transientData = Buffer.from(
      JSON.stringify({
        docID,
        docTitle,
        docData,
        price: parseInt(price),
      })
    );

    // Create a stateful transaction for adding the document
    let statefulTxn = contractOrg.createTransaction("AddDocument");
    statefulTxn.setTransient({
      asset_properties: transientData,
    });

    // Submit the transaction to the ledger
    try {
      await statefulTxn.submit();
      console.log(`Document added successfully for ID ${docID}\n`);
    } catch (error) {
      console.error(`Error adding document:\n`);
      throw error;
    }
  }
  else if (command === "QUERY_BALANCE") {
    /**
     * QUERY_BALANCE
     *
     * Query the balance of the user
     */
    try {
      const response = await contractOrg.evaluateTransaction("GetBalance");
      console.log(`Balance is ${response}\n`);
    } catch (error) {
      console.error("Error querying balance:\n");
      throw error;
    }
  } else if (command.startsWith("UPDATE_DOC_DATA")) {
    /**
     * UPDATE_DOC_DATA <docID> <newDocData> <updateHash>
     *
     * Update the data of a document
     * docID: Unique identifier of the document
     * newDocData: New data for the document
     * updateHash: Boolean flag to update the hash
     */

    const [_, docID, newDocData, updateHash] = command.split(" ");

    // Check if the parameters are provided
    if (!docID || !newDocData || !updateHash) {
      console.error("Please provide all the required parameters.\n");
      return;
    }

    // Validate input parameters
    if (updateHash.toLowerCase() !== "true" && updateHash.toLowerCase() !== "false") {
      console.error("Invalid updateHash parameter. Please provide 'true' or 'false'.\n");
      return;
    }

    // Convert updateHash from string to boolean
    const shouldUpdateHash = updateHash.toLowerCase() === "true";

    // Prepare transient data
    const transientData = {
      new_asset_properties: Buffer.from(JSON.stringify({ newDocData })),
    };


    let statefulTxn = contractOrg.createTransaction("UpdateDocument");
    // Set transient data in the context stub
    statefulTxn.setTransient(transientData);

    // Call the chaincode to update the document data
    try {
      await statefulTxn.submit(docID, shouldUpdateHash);
      console.log(`Document ${docID} updated successfully.`);
    } catch (error) {
      console.error(`Error updating document:`);
      throw error;
    }
  }
  else if (command === "GET_ALL_DOCS") {
    /**
     * GET_ALL_DOCS
     *
     * Get all documents from the ledger
     */
    const response = await contractOrg.evaluateTransaction("GetAllDocuments");
    const docs = JSON.parse(response.toString());
    if (docs.length !== 0) {
      console.log("All documents:");
      printDocList(docs);
    }
    else {
      console.log("No documents found\n");
    }
  } else if (command.startsWith("GET_DOC")) {
    /**
     * GET_DOC <docID>
     *
     * Get the document with the specified ID
     * docID: Unique identifier of the document
     */
    // check if the docID is provided
    if (!command.split(" ")[1]) {
      console.error("Please provide the document ID.\n");
      return;
    }

    const docID = command.split(" ")[1];
    const response = await contractOrg.evaluateTransaction(
      "GetDocument",
      docID
    );
    if (response.length !== 0) {
      console.log(`Document ${docID} details:`);
      printDocDetails(JSON.parse(response.toString()));
    } else {
      console.error(`Document with ID ${docID} not found.\n`);
    }
  } else {
    console.error(
      "Invalid command. Supported commands: ADD_MONEY, ADD_DOC, QUERY_BALANCE, UPDATE_DOC_DATA, GET_ALL_DOCS, GET_DOC, EXIT\n"
    );
  }
}

// Execute the main function
main();
