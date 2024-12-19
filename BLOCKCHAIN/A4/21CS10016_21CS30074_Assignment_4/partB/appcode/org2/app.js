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
const { buildWallet, buildCCPOrg2 } = require("../AppUtil.js");

// Create a readline interface
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
});

// Constants
const myChannel = "mychannel";
const myChaincodeName = "mychaincode";
const org2PrivateCollectionName = "Org2MSPPrivateCollection";
const mspOrg2 = "Org2MSP";
const Org2UserId = "appUser2"

const RED = '\x1b[31m\n';
const GREEN = '\x1b[32m\n';
const RESET = '\x1b[0m';
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

/**
 * Function to initialize the contract using the Org2 identity
 * 
 * @returns {Promise<Gateway>} The gateway object
 * @throws {Error} If an error occurs
 */
async function initContractFromOrg2Identity() {
    console.log(
        "\n--> Fabric client user & Gateway init: Using Org2 identity to Org2 Peer"
    );
    const ccpOrg2 = buildCCPOrg2();
    const caOrg2Client = buildCAClient(
        FabricCAServices,
        ccpOrg2,
        "ca.org2.example.com"
    );
    const walletPathOrg2 = path.join(process.cwd(), "wallet/org2");
    const walletOrg2 = await buildWallet(Wallets, walletPathOrg2);

    identity = await walletOrg2.get(Org2UserId);

    await enrollAdmin(caOrg2Client, walletOrg2, mspOrg2);
    await registerAndEnrollUser(caOrg2Client, walletOrg2, mspOrg2, Org2UserId, "org2.department1");

    try {
        const gatewayOrg2 = new Gateway();
        await gatewayOrg2.connect(ccpOrg2, {
            wallet: walletOrg2,
            identity: Org2UserId,
            discovery: { enabled: true, asLocalhost: true },
        });
        return gatewayOrg2;
    } catch (error) {
        console.error(`Error in connecting to gateway: ${error}`);
        process.exit(1);
    }
}

// Main function for the terminal-based application
async function main() {
    let gatewayOrg2;
    try {
        let listener = null;
        let wishlist = new Set();
        let already_in_the_market = new Set();
        gatewayOrg2 = await initContractFromOrg2Identity();
        const networkOrg2 = await gatewayOrg2.getNetwork(myChannel);
        const contractOrg2 = networkOrg2.getContract(myChaincodeName);

        try {
            listener = async (event) => {
                const asset = JSON.parse(event.payload.toString());
                if (event.eventName === 'AddToMarket' && wishlist.has(asset.docID)) {
                    try {
                        const doc_bought = await contractOrg2.submitTransaction('BuyFromMarket', asset.docID);
                        // Check if doc_bought is empty
                        if (!doc_bought || doc_bought.length === 0) {
                            console.log(`Error in buying Document with ID ${asset.docID}`);
                            console.error(`${RED}Document with ID ${asset.docID} has been tampared${RESET}`);
                        }
                        else {
                            console.log(`Item ${asset.docID} bought from the marketplace`);
                            wishlist.delete(asset.docID);
                        }
                    } catch (error) {
                        console.error(`Error in buying item from the marketplace: ${error}`);
                        if (error.message) {
                            console.error(`${RED}${error.message}${RESET}`);\
                        }
                        already_in_the_market.add(asset.docID);
                    }
                }
                else if (event.eventName === 'AddToMarket' && !wishlist.has(asset.docID)) {
                    already_in_the_market.add(asset.docID);
                }
                else if (event.eventName === 'BuyFromMarket') {
                    already_in_the_market.delete(asset.docID);
                }
            };
            console.log(`${GREEN}--> Start contract event stream to peer in Org2${RESET}`);
            await contractOrg2.addContractListener(listener);
        } catch (eventError) {
            console.log(`${RED}<-- Failed: Setup contract events - ${eventError}${RESET}`);
        }

        // Add discovery interest
        contractOrg2.addDiscoveryInterest({
            name: myChaincodeName,
            collectionNames: [org2PrivateCollectionName],
        });
        try {
            if (identity == null) {
                await contractOrg2.submitTransaction("InitLedger");
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
7. ENLIST_DOC <docID> - Enlists the document with the specified ID in the marketplace
8. WISHLIST <docID> - Adds the document with the specified ID to the wishlist
9. ALL_DOCS - Retrieves all documents enlisted in the marketplace
10. ALL_RECORDS - Retrieves all transaction records
11. EXIT - Exits the application

Please enter your command:
`);
                if (command === "EXIT") {
                    console.log("Exiting the application...");
                    break;
                }

                try {
                    await handleCommand(command, contractOrg2, wishlist, already_in_the_market);
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
                await gatewayOrg2.disconnect();
            }
            catch (error) {
                console.error(`Error in disconnecting gateway: ${error}`);
            }
        }
    }
    catch (error) {
        console.error(`Error in main function: ${error}`);
        process.exit(1);
    }
}


/**
 * Function to handle the user command
 * 
 * @param {string} command The user command
 * @param {Contract} contractOrg The contract object for Org
 * @param {Set<string>} wishlist The wishlist of the user
 * @param {Set<string>} already_in_the_market The set of documents already in the market
 * @returns {Promise<void>}
 * @throws {Error} If an error occurs
 */
async function handleCommand(command, contractOrg, wishlist, already_in_the_market) {
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
                price: parseInt(price)
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
    }
    else if (command.startsWith("ENLIST_DOC")) {
        /**
         * ENLIST_DOC <docID>
         * 
         * Enlist the document with the specified ID in the marketplace
         * docID: Unique identifier of the document
         */
        // Check if the docID is provided
        if (!command.split(" ")[1]) {
            console.error("Please provide the document ID.\n");
            return;
        }

        const docID = command.split(" ")[1];

        let statefulTxn = contractOrg.createTransaction("AddToMarket");
        try {
            const response = await statefulTxn.submit(docID);
            // If response is empty, the document did not exist in the private data collection
            if (!response || response.length === 0) {
                console.error(`Document with ID ${docID} not found in the private data collection.\n`);
                return;
            }
            console.log(`Document ${docID} enlisted successfully.\n`);
        } catch (error) {
            console.error(`Error enlisting document:\n`);
            throw error;
        }

    }
    else if (command.startsWith("ALL_DOCS")) {
        /**
         * ALL_DOCS
         * 
         * Get all documents enlisted in the marketplace
         */
        const response = await contractOrg.evaluateTransaction("GetDocsInMarket");
        const docs = JSON.parse(response.toString());
        if (docs.length !== 0) {
            console.log("All documents enlisted in the marketplace:\n");
            printMarketPlaceDocs(docs);
        }
        else {
            console.log("No documents enlisted in the marketplace\n");
        }
    }
    else if (command.startsWith("WISHLIST")) {
        /**
         * WISHLIST <docID>
         * 
         * Add the document with the specified ID to the wishlist
         * docID: Unique identifier of the document
         */
        // Check if the docID is provided
        if (!command.split(" ")[1]) {
            console.error("Please provide the document ID.\n");
            return;
        }

        const docID = command.split(" ")[1];
        wishlist.add(docID);
        console.log(`Document ${docID} added to the wishlist.\n`);

        if (already_in_the_market.has(docID)) {
            try {
                const doc_bought = await contractOrg.submitTransaction('BuyFromMarket', docID);
                // Check if doc_bought is empty
                if (!doc_bought || doc_bought.length === 0) {
                    console.log(`Error in buying Document with ID ${docID}`);
                    console.error(`${RED}Document with ID ${docID} has been tampared${RESET}`);
                }
                else {
                    console.log(`Item ${docID} bought from the marketplace`);
                    wishlist.delete(docID);
                }
            } catch (error) {
                console.error(`Error in buying item from the marketplace: ${error}`);
                if (error.message) {
                    console.error(`${RED}${error.message}${RESET}`);
                }
                already_in_the_market.add(docID);
            }
        }
    }
    else if (command === "ALL_RECORDS") {
        /**
         * ALL_RECORDS
         * 
         * Get all transaction records
         */
        const response = await contractOrg.evaluateTransaction("GetRecordsInMarket");
        const records = JSON.parse(response.toString());
        if (records.length !== 0) {
            console.log("All transaction records:\n");
            printMarketPlaceRecords(records);
        }
        else {
            console.log("No transaction records found\n");
        }
    }
    else {
        console.error(
            "Invalid command. Supported commands: ADD_MONEY, ADD_DOC, QUERY_BALANCE, UPDATE_DOC_DATA, GET_ALL_DOCS, GET_DOC, ENLIST_DOC, WISHLIST, ALL_DOCS, ALL_RECORDS, EXIT\n"
        );
    }

}

/**
 * Function to print the details of the documents in the marketplace
 * 
 * @param {Array<Object>} docs The list of documents
 * @returns {void}
 */
function printMarketPlaceDocs(docs) {
    docs.forEach((doc) => {
        // Get the Key and Record from the doc
        const { Key, Record } = doc;
        // Remove the prefix from the Key
        const docID = Key.split("_")[1];
        // Print the details of the document
        console.log(`Document ID: ${docID}`);
        console.log(`Seller: ${Record.seller}`);
        console.log(`Document Title: ${Record.docTitle}`);
        console.log(`Document Data: ${Record.docData}`);
        console.log(`Document Price: ${Record.price}`);
        console.log(`Document Hash: ${Record.docHash}`);
        console.log("\n");
    });
}

/**
 * Function to print the transaction records in the marketplace
 * 
 * @param {Array<Object>} records The list of records
 * @returns {void}
 */
function printMarketPlaceRecords(records) {
    records.forEach((record) => {
        // Get the Key and Record from the doc
        const { Key, Record } = record;
        console.log(`Document ID: ${Record.docID}`);
        console.log(`Seller: ${Record.seller}`);
        console.log(`Buyer: ${Record.buyer}`);
        console.log(`Price: ${Record.docPrice}`);
        console.log(`Document Hash: ${Record.docDataHash}`);
        console.log("\n");
    });
}

// Execute the main function
main();
