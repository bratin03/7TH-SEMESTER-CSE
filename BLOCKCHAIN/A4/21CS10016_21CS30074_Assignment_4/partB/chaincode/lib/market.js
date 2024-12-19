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

const { Contract } = require("fabric-contract-api");
const crypto = require("crypto");

class MarketOps extends Contract {

    /**
     * Initializes the ledger with the caller's MSP ID and a balance of 0.
     *
     * @param {Object} ctx - The context containing stub methods for interacting with the ledger.
     * @returns {null} - Returns null upon successful initialization of the ledger.
     * @throws {Error} - Throws an error if the ledger cannot be initialized.
     */
    async InitLedger(ctx) {
        const callerMSPID = ctx.clientIdentity.getMSPID();

        // Initialize balance data
        const initialData = { balance: 0 };

        try {
            // Store the initial balance in the ledger
            await ctx.stub.putState(
                callerMSPID,
                Buffer.from(JSON.stringify(initialData))
            );
        } catch (error) {
            // Handle errors and throw a new error for further handling
            throw new Error(`Failed to initialize ledger for MSP ${callerMSPID}: ${error.message}`);
        }

        return null; // Successful operation
    }

    /**
     * Calculates the SHA-256 hash of the specified data.
     * 
     * @param {string} data - The data to hash.
     * @returns The SHA-256 hash of the data.
     */
    hashData(data) {
        return crypto.createHash('sha256').update(data).digest('hex');
    }


    /**
     * Adds the specified amount to the balance of the caller's account.
     *
     * @param {Object} ctx - The context containing stub methods for interacting with the ledger.
     * @returns {number} The new balance of the caller's account.
     * @throws {Error} Throws an error if the balance cannot be updated.
     */
    async AddBalance(ctx) {
        const callerMSPID = ctx.clientIdentity.getMSPID();
        const balanceBytes = await ctx.stub.getState(callerMSPID);

        // Check if the account has been initialized
        if (!balanceBytes || balanceBytes.length === 0) {
            throw new Error(`${callerMSPID} not initialized`);
        }

        // Retrieve the amount from transient data
        const transientData = ctx.stub.getTransient();
        const amountBuffer = transientData.get("amount");

        // Validate the presence of the amount
        if (!amountBuffer || amountBuffer.length === 0) {
            throw new Error("Amount not found in transient data");
        }

        let amount;
        try {
            // Parse the amount from transient data
            const amountData = JSON.parse(amountBuffer.toString());
            amount = amountData.amount; // Access the amount field directly
        } catch (error) {
            throw new Error(`Failed to parse amount from transient data: ${error.message}`);
        }

        // Validate the amount
        this._validateAmount(amount);

        // Update the balance
        let balance;
        try {
            balance = JSON.parse(balanceBytes.toString());
        } catch (error) {
            throw new Error(`Failed to parse existing balance: ${error.message}`);
        }

        // Validate the balance structure
        this._validateBalanceStructure(balance);

        // Update the balance
        balance.balance += amount;

        // Save the new balance back to the ledger
        await ctx.stub.putState(callerMSPID, Buffer.from(JSON.stringify(balance)));

        // Return the updated balance
        return balance.balance;
    }

    /**
    * Returns the balance of the caller's account.
    *
    * @param {Object} ctx - The context containing stub methods for interacting with the ledger.
    * @returns {number} The balance of the caller's account.
    * @throws {Error} Throws an error if the balance cannot be retrieved.
    */
    async GetBalance(ctx) {
        const callerMSPID = ctx.clientIdentity.getMSPID();

        // Check if the caller's MSP ID is valid
        if (!callerMSPID) {
            throw new Error("Caller MSP ID is not valid");
        }

        // Retrieve the balance bytes from the ledger
        const balanceBytes = await ctx.stub.getState(callerMSPID);

        // Check if the balance exists in the state
        if (!balanceBytes || balanceBytes.length === 0) {
            throw new Error(`${callerMSPID} not initialized`);
        }

        // Parse the balance data
        let balanceData;
        try {
            balanceData = JSON.parse(balanceBytes.toString());
        } catch (error) {
            throw new Error(`Failed to parse balance data: ${error.message}`);
        }

        // Validate the parsed data
        this._validateParsedBalanceData(balanceData);

        // Return the balance
        return balanceData.balance;
    }

    /**
    * Validates the specified amount.
    *
    * @param {number} amount - The amount to validate.
    * @throws {Error} Throws an error if the amount is invalid.
    * @private
    */
    _validateAmount(amount) {
        if (typeof amount !== "number" || isNaN(amount)) {
            throw new Error("Invalid amount: must be a number");
        }
        if (amount < 0) {
            throw new Error("Invalid amount: must be non-negative");
        }
    }

    /**
    * Validates the structure of the balance object.
    *
    * @param {Object} balance - The balance object to validate.
    * @throws {Error} Throws an error if the balance structure is invalid.
    * @private
    */
    _validateBalanceStructure(balance) {
        if (typeof balance !== "object" || balance === null || typeof balance.balance !== "number") {
            throw new Error("Invalid balance structure: missing or malformed balance");
        }
    }

    /**
    * Validates the parsed balance data.
    *
    * @param {Object} balanceData - The balance data to validate.
    * @throws {Error} Throws an error if the balance data is invalid.
    * @private
    */
    _validateParsedBalanceData(balanceData) {
        if (typeof balanceData.balance === "undefined") {
            throw new Error("Balance field is missing in the balance data");
        }
        if (typeof balanceData.balance !== "number" || isNaN(balanceData.balance)) {
            throw new Error("Invalid balance value: it must be a number");
        }
    }

    /**
     * Adds a new document to the private data collection.
     *
     * @param {Object} ctx - The context containing stub methods for interacting with the ledger.
     * @returns {null} - Returns null upon successful addition of the document.
     * @throws {Error} - Throws an error if the document cannot be added.
     */
    async AddDocument(ctx) {
        // Verify that the client is submitting a request to a peer in their organization
        const orgMismatchError = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (orgMismatchError !== null) {
            throw new Error(`Operation not permitted: ${orgMismatchError}`);
        }

        // Retrieve the transient map and validate presence of asset properties
        const transientMap = ctx.stub.getTransient();
        const transientAssetJSON = transientMap.get("asset_properties");

        if (!transientAssetJSON || transientAssetJSON.length === 0) {
            throw new Error("Missing asset properties in the transient map");
        }

        // Parse the transient asset JSON
        let jsonFromString;
        try {
            jsonFromString = JSON.parse(transientAssetJSON.toString());
        } catch (err) {
            throw new Error(`Error parsing asset properties: ${err.message}`);
        }

        // Validate required document fields
        this._validateDocumentFields(jsonFromString);

        // Calculate hash of the document data
        const docHash = this.hashData(jsonFromString.docData);

        // Retrieve the collection name for the organization
        let orgCollection;
        try {
            orgCollection = await this._getCollectionName(ctx);
            if (!orgCollection) {
                throw new Error("Unable to retrieve the collection name for the organization");
            }
        } catch (error) {
            throw new Error(`Error retrieving collection name: ${error.message}`);
        }

        // Check for existence of the document
        try {
            const assetAsBytes = await ctx.stub.getPrivateData(orgCollection, jsonFromString.docID);
            if (assetAsBytes && assetAsBytes.length > 0) {
                throw new Error(`Document already exists: ${jsonFromString.docID}`);
            }
        } catch (error) {
            throw new Error(`Error checking document existence: ${error.message}`);
        }

        // Create an object with document data and its hash
        const documentWithHash = {
            ...jsonFromString,
            docHash: docHash, // Include the hash in the object
        };

        // Store the document in the private data collection
        try {
            await ctx.stub.putPrivateData(
                orgCollection,
                jsonFromString.docID,
                Buffer.from(JSON.stringify(documentWithHash))
            );
        } catch (error) {
            throw new Error(`Failed to store document: ${error.message}`);
        }

        return null; // Successful operation
    }

    /**
    * Validates the required fields of the document.
    *
    * @param {Object} jsonFromString - The document data to validate.
    * @throws {Error} - Throws an error if validation fails.
    * @private
    */
    _validateDocumentFields(jsonFromString) {
        if (!jsonFromString.docID || typeof jsonFromString.docID !== 'string') {
            throw new Error("Document ID field must be a non-empty string");
        }
        if (!jsonFromString.docTitle || typeof jsonFromString.docTitle !== 'string') {
            throw new Error("Document Title field must be a non-empty string");
        }
        if (!jsonFromString.docData || typeof jsonFromString.docData !== 'string') {
            throw new Error("Document Data field must be a non-empty string");
        }
        if (typeof jsonFromString.price !== 'number' || jsonFromString.price <= 0) {
            throw new Error("Price must be a positive number");
        }
    }


    /**
     * UpdateDocument updates the document data in the private data collection.
     *
     * @param {context} ctx - The context containing stub methods for interacting with the ledger.
     * @param {string} docID - The ID of the document to update.
     * @param {string} updateHash - A boolean indicating whether to update the document hash.
     * @returns {null} - Returns null upon successful update of the document.
     * @throws {Error} - Throws an error if the document cannot be updated.
     */
    async UpdateDocument(ctx, docID, updateHash) {
        // Verify that the client is submitting a request to a peer in their organization
        const orgMismatchError = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (orgMismatchError !== null) {
            throw new Error(`Operation not permitted: ${orgMismatchError}`);
        }

        // Get the transient data containing the new document data
        const transientMap = ctx.stub.getTransient();
        const transientNewDocData = transientMap.get("new_asset_properties");

        if (!transientNewDocData || transientNewDocData.length === 0) {
            throw new Error("New document data not found in the transient map");
        }

        let jsonFromString;
        try {
            const jsonBytesToString = String.fromCharCode(...transientNewDocData);
            jsonFromString = JSON.parse(jsonBytesToString);
        } catch (error) {
            throw new Error(`Error parsing new document data: ${error.message}`);
        }

        // Validate new document data
        if (!jsonFromString.newDocData || jsonFromString.newDocData.length === 0) {
            throw new Error("New Document Data field must be a non-empty string");
        }

        // Get collection name for this organization
        let orgCollection;
        try {
            orgCollection = await this._getCollectionName(ctx);
        } catch (error) {
            throw new Error(`Error retrieving collection name: ${error.message}`);
        }

        // Check if the document exists
        let assetAsBytes;
        try {
            assetAsBytes = await ctx.stub.getPrivateData(orgCollection, docID);
            if (!assetAsBytes || assetAsBytes.length === 0) {
                throw new Error("Document does not exist: " + docID);
            }
        } catch (error) {
            throw new Error(`Error checking document existence: ${error.message}`);
        }

        // Parse the existing document details
        let existingDocument;
        try {
            existingDocument = JSON.parse(assetAsBytes.toString());
        } catch (error) {
            throw new Error(`Error parsing existing document data: ${error.message}`);
        }

        // Update the document data
        existingDocument.docData = jsonFromString.newDocData; // Update document data

        // Declare a boolean to determine if the hash should be updated
        let updateHash_bool;

        // Convert the updateHash parameter to a boolean
        updateHash_bool = updateHash.toLowerCase() === "true";

        // Compute a new hash if requested
        if (updateHash_bool) {
            const newDocHash = this.hashData(existingDocument.docData); // Compute new hash
            existingDocument.docHash = newDocHash; // Update the stored document hash
        }

        // Store the updated document in the private data collection
        try {
            await ctx.stub.putPrivateData(
                orgCollection,
                docID,
                Buffer.from(JSON.stringify(existingDocument))
            );
        } catch (error) {
            throw new Error(`Failed to store updated document: ${error.message}`);
        }

        return null; // Successful operation
    }

    /**
     * GetAllDocuments retrieves all documents from the private data collection.
     * 
     * @param {Context} ctx - The context containing methods for interacting with the ledger.
     * @returns {string} - A JSON string containing all documents in the private data collection.
     * @throws {Error} - Throws an error if the documents cannot be retrieved.
     */
    async GetAllDocuments(ctx) {
        // Check if the client is authorized to access the private data
        const orgMismatchError = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (orgMismatchError !== null) {
            throw new Error(`Operation not permitted: ${orgMismatchError}`);
        }
        let orgCollection;
        try {
            // Get collection name for this organization
            orgCollection = await this._getCollectionName(ctx);
        } catch (error) {
            throw new Error(`Error retrieving collection name: ${error.message}`);
        }

        // Retrieve all documents from the private data collection
        let response;
        try {
            response = await ctx.stub.getPrivateDataByRange(orgCollection, "", "");
        } catch (error) {
            throw new Error(`Error retrieving documents: ${error.message}`);
        }

        let results = await this._getAllResults(response.iterator);

        return JSON.stringify(results);
    }

    /**
     * Get all results from an iterator and return them in an array.
     * 
     * @param {Object} iterator - The iterator to retrieve results from.
     * @returns {Array} - An array containing all results from the iterator.
     */
    async _getAllResults(iterator) {
        const allResults = [];
        while (true) {
            const res = await iterator.next();
            if (res.value) {
                // Parse the value directly to an object and push to results
                allResults.push(JSON.parse(res.value.value.toString('utf8')));
            }

            // Check to see if we have reached the end
            if (res.done) {
                // Explicitly close the iterator            
                await iterator.close();
                return allResults;
            }
        }
    }


    /**
     * GetDocument retrieves the details of a document by its ID from the private data collection.
     *
     * @param {context} ctx - The context containing stub methods for interacting with the ledger.
     * @param {string} docID - The ID of the document to retrieve
     * @returns {Object} - The details of the document.
     * @throws {Error} - Throws an error if the document cannot be found or retrieved.
     */
    async GetDocument(ctx, docID) {
        // Check if the client is authorized to access the private data
        const orgMismatchError = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (orgMismatchError !== null) {
            throw new Error(`Operation not permitted: ${orgMismatchError}`);
        }
        // Get collection name for this organization
        let orgCollection;
        try {
            orgCollection = await this._getCollectionName(ctx);
        } catch (error) {
            throw new Error(`Error retrieving collection name: ${error.message}`);
        }

        // Retrieve the document from the private data collection
        let assetAsBytes;
        try {
            assetAsBytes = await ctx.stub.getPrivateData(orgCollection, docID);
            if (!assetAsBytes || assetAsBytes.length === 0) {
                // Return empty object if the document does not exist
                return null;
            }
        } catch (error) {
            throw new Error(`Error retrieving document: ${error.message}`);
        }

        // Parse and return the document details
        let document;
        try {
            document = JSON.parse(assetAsBytes.toString());
        } catch (error) {
            throw new Error(`Error parsing document data: ${error.message}`);
        }

        // Return the document details in a structured format
        return {
            docTitle: document.docTitle,
            docData: document.docData,
            docHash: document.docHash,
            price: document.price,
        };
    }


    /**
     * Verifies that the client's organization ID matches the peer organization ID.
     * 
     * @param {Context} ctx - The context containing methods for interacting with the ledger, including access to the client identity and the ledger state.
     * @returns {null} - Returns null if the organization IDs match.
     * @throws {Error} - Throws an error if the organization IDs do not match.
     */
    async _verifyClientOrgMatchesPeerOrg(ctx) {
        const clientMSPID = ctx.clientIdentity.getMSPID();
        const peerMSPID = ctx.stub.getMspID();

        if (clientMSPID !== peerMSPID) {
            throw new Error(`Client from organization ${clientMSPID} is not authorized to access private data from organization ${peerMSPID}`);
        }

        return null;
    }


    /**
     * Retrieves the private data collection name for the submitting client's organization.
     * 
     * @param {Context} ctx - The context containing methods for interacting with the ledger.
     * @returns {string} - The name of the private data collection.
     */
    async _getCollectionName(ctx) {
        // Get the MSP ID of the submitting client identity
        const clientMSPID = ctx.clientIdentity.getMSPID();
        // Construct the collection name
        const orgCollection = `${clientMSPID}PrivateCollection`;

        return orgCollection;
    }


    /**
     * Adds a document to the market.
     * 
     * @param {Context} ctx - The context containing stub methods for interacting with the ledger.
     * @param {string} docID - The ID of the document to add to the market.
     * @returns {string} - The document added to the market.
     * @throws {Error} - Throws an error if the document cannot be added to the market.
     */
    async AddToMarket(ctx, docID) {
        // Check if the client is authorized to access the private data
        const orgMismatchError = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (orgMismatchError !== null) {
            throw new Error(`Operation not permitted: ${orgMismatchError}`);
        }
        const orgCollection = await this._getCollectionName(ctx);
        const assetAsBytes = await ctx.stub.getPrivateData(orgCollection, docID);
        // No Asset found, return empty response
        if (!assetAsBytes || assetAsBytes.length === 0) {
            return null;
        }


        // Check if the document already exists in the market
        const itemBytes = await ctx.stub.getState("doc_" + docID);
        if (itemBytes && itemBytes.length > 0) {
            throw new Error(`Document already exists in the market: ${docID}`);
        }

        // Add the seller info to the document
        const sellerMSPID = ctx.clientIdentity.getMSPID();
        // Add this to the document
        const item = JSON.parse(assetAsBytes.toString());
        item.seller = sellerMSPID;
        // Add the document to the market with the seller info
        const assetAsBytes_ = Buffer.from(JSON.stringify(item));

        await ctx.stub.putState("doc_" + docID, assetAsBytes_);

        ctx.stub.setEvent('AddToMarket', assetAsBytes);

        return String.fromCharCode(...assetAsBytes);
    }

    /**
     * Buys a document from the market.
     * 
     * @param {Context} ctx - The context containing stub methods for interacting with the ledger.
     * @param {string} docID - The ID of the document to buy from the market.
     * @returns {null} - Returns null upon successful purchase of the document.
     * @throws {Error} - Throws an error if the document cannot be bought from the market.
     */
    async BuyFromMarket(ctx, docID) {
        const callerMSPID = ctx.clientIdentity.getMSPID();
        const balanceBytes = await ctx.stub.getState(callerMSPID);
        if (!balanceBytes || balanceBytes.length === 0) {
            throw new Error(`${callerMSPID} not initialised`);
        }
        let balance = JSON.parse(balanceBytes.toString()).balance;

        const itemBytes = await ctx.stub.getState("doc_" + docID);
        if (!itemBytes || itemBytes.length === 0) {
            throw new Error(`${docID} not found in market`);
        }

        const item = JSON.parse(itemBytes.toString());

        const hashCalcualted = this.hashData(item.docData);
        if (hashCalcualted !== item.docHash) {
            // Remove the item from the market
            await ctx.stub.deleteState("doc_" + docID);
            return null;
        }

        if (item.price > balance) {
            throw new Error(`Insufficient funds. Required ${item.price} but you have only ${balance} balance. `);
        }

        const sellerMSPID = item.seller;
        const sellerBalanceBytes = await ctx.stub.getState(sellerMSPID);
        if (!sellerBalanceBytes || sellerBalanceBytes.length === 0) {
            throw new Error(`${sellerMSPID} not initialised`);
        }

        let sellerBalance = JSON.parse(sellerBalanceBytes.toString()).balance;
        sellerBalance += item.price;
        await ctx.stub.putState(sellerMSPID, Buffer.from(JSON.stringify({ balance: sellerBalance })));

        balance -= item.price;
        await ctx.stub.putState(callerMSPID, Buffer.from(JSON.stringify({ balance: balance })));
        await ctx.stub.deleteState("doc_" + docID);

        // Create a transaction record
        const transaction = {
            buyer: callerMSPID,
            seller: sellerMSPID,
            docID: docID,
            docPrice: item.price,
            docDataHash: item.docHash,
        };

        ctx.stub.setEvent('BuyFromMarket', Buffer.from(JSON.stringify(transaction)));

        // Create a key for the transaction
        const transactionKey = `transaction_${docID}_${callerMSPID}_${sellerMSPID}_${Date.now()}`;

        // Store the transaction in the ledger
        await ctx.stub.putState(transactionKey, Buffer.from(JSON.stringify(transaction)));

        const err = await this._verifyClientOrgMatchesPeerOrg(ctx);
        if (err !== null) {
            throw new Error('CreateAsset cannot be performed: Error ' + err);
        }

        // Get collection name for this organization.
        const orgCollection = await this._getCollectionName(ctx);
        await ctx.stub.putPrivateData(orgCollection, item.docID, Buffer.from(JSON.stringify(item)));
        // Return the docID of the bought document
        return docID;
    }

    /**
     * Get all documents in the market.
     * 
     * @param {Context} ctx - The context containing stub methods for interacting with the ledger.
     * @returns {string} - A JSON string containing all documents in the market.
     * @throws {Error} - Throws an error if the documents cannot be retrieved from the market.
     */
    async GetDocsInMarket(ctx) {
        return await this._GetAssetsByRange(ctx, 'doc_', 'doc_`');
    }

    /**
     * Get all transactions in the market.
     * 
     * @param {Context} ctx - The context containing stub methods for interacting with the ledger.
     * @returns {string} - A JSON string containing all transactions in the market.
     * @throws {Error} - Throws an error if the transactions cannot be retrieved from the market.
     */
    async GetRecordsInMarket(ctx) {
        return await this._GetAssetsByRange(ctx, 'transaction_', 'transaction_`');
    }

    /**
     * Get all assets in the ledger that fall within the specified key range.
     * 
     * @param {Context} ctx - The context containing stub methods for interacting with the ledger.
     * @param {string} startKey - The start key of the range.
     * @param {string} endKey - The end key of the range.
     * @returns {string} - A JSON string containing all assets in the specified range.  
     */
    async _GetAssetsByRange(ctx, startKey, endKey) {

        let resultsIterator = await ctx.stub.getStateByRange(startKey, endKey);
        let results = await this._GetAllResults(resultsIterator, false);

        return JSON.stringify(results);
    }

    /**
     * Get all results from an iterator and return them in an array.
     * 
     * @param {Object} iterator - The iterator to retrieve results from.
     * @param {boolean} isHistory - A boolean indicating whether the iterator is for history results.
     * @returns {Array} - An array containing all results from the iterator.
     * @throws {Error} - Throws an error if the results cannot be retrieved.
     */ 
    async _GetAllResults(iterator, isHistory) {
        let allResults = [];
        let res = await iterator.next();
        while (!res.done) {
            if (res.value && res.value.value.toString()) {
                let jsonRes = {};
                console.log(res.value.value.toString('utf8'));
                if (isHistory && isHistory === true) {
                    jsonRes.TxId = res.value.txId;
                    jsonRes.Timestamp = res.value.timestamp;
                    try {
                        jsonRes.Value = JSON.parse(res.value.value.toString('utf8'));
                    } catch (err) {
                        console.log(err);
                        jsonRes.Value = res.value.value.toString('utf8');
                    }
                } else {
                    jsonRes.Key = res.value.key;
                    try {
                        jsonRes.Record = JSON.parse(res.value.value.toString('utf8'));
                    } catch (err) {
                        console.log(err);
                        jsonRes.Record = res.value.value.toString('utf8');
                    }
                }
                allResults.push(jsonRes);
            }
            res = await iterator.next();
        }
        iterator.close();
        return allResults;
    }
}

module.exports = MarketOps;