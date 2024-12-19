const BlockCart = artifacts.require("BlockCart");

contract("BlockCart", (accounts) => {
    let blockCart;
    const [owner, buyer1, buyer2, buyer3, buyer4, buyer5, buyer6] = accounts;
    const productIds = [1, 2, 3];
    const productNames = ["Product 1", "Product 2", "Product 3"];
    const productPrices = [web3.utils.toWei('1', 'ether'), web3.utils.toWei('2', 'ether'), web3.utils.toWei('3', 'ether')];
    const inventory = 100;

    before(async () => {
        blockCart = await BlockCart.deployed();
    });

    it("should check the owner is set correctly", async () => {
        const contractOwner = await blockCart.owner();
        assert.equal(contractOwner, owner, "Owner is not set correctly");
    });

    it("should add 2 products and verify they are added correctly", async () => {
        for (let i = 0; i < 2; i++) {
            console.log(`Adding product ${productIds[i]}: ${productNames[i]}, ${inventory}, ${productPrices[i]}`);
            await blockCart.addProduct(productIds[i], productNames[i], inventory, productPrices[i], { from: owner });
            const product = await blockCart.products(productIds[i]);
            console.log(`Product ${productIds[i]}: Name=${product.name}, Inventory=${product.inventory}, Price=${product.price}`);
            assert.equal(product.name, productNames[i], `Product name mismatch for ID ${productIds[i]}`);
            assert.equal(product.inventory, inventory, `Product inventory mismatch for ID ${productIds[i]}`);
            assert.equal(product.price, productPrices[i], `Product price mismatch for ID ${productIds[i]}`);
        }
    });

    it("should update a product", async () => { // Need to pass id,name,inventory,price
        await blockCart.updateProduct(productIds[1],productNames[1],inventory,web3.utils.toWei('20', 'ether'), { from: owner });
        // No new product would be emitted, so we need to call the contract to get the updated product
        const product = await blockCart.products(productIds[1]);
        assert.equal(product.price, web3.utils.toWei('20', 'ether'), "Product price mismatch after update");
    });

    it("Empty product name should not be allowed", async () => {
        try {
            await blockCart.addProduct(productIds[2], "", inventory, productPrices[2], { from: owner });
            assert.fail("Empty product name was allowed. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should not allow a non-owner to add a product", async () => {
        try {
            await blockCart.addProduct(productIds[2], productNames[2], inventory, productPrices[2], { from: buyer1 });
            assert.fail("Non-owner was able to add a product. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should not allow a non-owner to update a product", async () => {
        try {
            await blockCart.updateProduct(productIds[1], productNames[1], inventory, productPrices[1]+2, { from: buyer1 });
            assert.fail("Non-owner was able to update a product. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });


    it("should register 5 new buyers and verify all are registered correctly", async () => {
        const buyerDetails = [
            { name: "Buyer 1", email: "buyer1@example.com", address: "123 Main St" },
            { name: "Buyer 2", email: "buyer2@example.com", address: "456 Elm St" },
            { name: "Buyer 3", email: "buyer3@example.com", address: "789 Oak St" },
            { name: "Buyer 4", email: "buyer4@example.com", address: "101 Pine St" },
            { name: "Buyer 5", email: "buyer5@example.com", address: "202 Maple St" }
        ];

        for (let i = 0; i < 5; i++) {
            console.log(`Registering buyer ${accounts[i + 1]}: ${buyerDetails[i].name}, ${buyerDetails[i].email}, ${buyerDetails[i].address}`);
            await blockCart.registerBuyer(buyerDetails[i].name, buyerDetails[i].email, buyerDetails[i].address, { from: accounts[i + 1] });
            const buyer = await blockCart.buyers(accounts[i + 1]);
            console.log(`Buyer ${accounts[i + 1]}: Name=${buyer.name}, Email=${buyer.email}, Address=${buyer.mailingAddress}`);
            assert.equal(buyer.name, buyerDetails[i].name, `Buyer name mismatch for account ${accounts[i + 1]}`);
            assert.equal(buyer.email, buyerDetails[i].email, `Buyer email mismatch for account ${accounts[i + 1]}`);
            assert.equal(buyer.mailingAddress, buyerDetails[i].address, `Buyer address mismatch for account ${accounts[i + 1]}`);
        }
    });

    it("should allow a registered buyer to buy a product and update inventory", async () => {
        const initialBalance = BigInt(await web3.eth.getBalance(buyer1));
        console.log(`Initial buyer1 balance: ${initialBalance}`);

        console.log(`Buyer ${buyer1} buying product ${productIds[0]} with quantity 2`);
        await blockCart.buyProduct(productIds[0], 2, { from: buyer1, value: web3.utils.toWei('2', 'ether') });
        const product = await blockCart.products(productIds[0]);
        console.log(`Product ${productIds[0]}: New inventory=${product.inventory}`);
        assert.equal(product.inventory, 98, "Product inventory mismatch after purchase");

        const afterBalance = BigInt(await web3.eth.getBalance(buyer1));
        console.log(`After buyer1 balance: ${afterBalance}`);
    });

    it("should not allow a registered buyer to buy a non-existent product", async () => {
        try {
            console.log(`Buyer ${buyer1} attempting to buy a non-existent product`);
            await blockCart.buyProduct(0, 1, { from: buyer1, value: web3.utils.toWei('1', 'ether') });
            assert.fail("Buyer was able to buy a non-existent product. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should not allow an unregistered buyer to buy a product", async () => {
        try {
            console.log(`Buyer ${buyer6} attempting to buy product ${productIds[0]} with quantity 2`);
            await blockCart.buyProduct(productIds[0], 2, { from: buyer6, value: web3.utils.toWei('2', 'ether') });
            assert.fail("Unregistered buyer was able to buy a product. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should handle buying more products than available in inventory", async () => {
        try {
            console.log(`Buyer ${buyer1} attempting to buy more products than available`);
            await blockCart.buyProduct(productIds[0], 200, { from: buyer1, value: web3.utils.toWei('200', 'ether') });
            assert.fail("Buyer was able to buy more products than available in inventory. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should handle zero quantity purchase", async () => {
        try {
            console.log(`Buyer ${buyer1} attempting to buy product ${productIds[0]} with zero quantity`);
            await blockCart.buyProduct(productIds[0], 0, { from: buyer1, value: web3.utils.toWei('0', 'ether') });
            assert.fail("Buyer was able to buy a product with zero quantity. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it ("should handle payment less than product price", async () => {
        try {
            console.log(`Buyer ${buyer1} attempting to buy product ${productIds[0]} with insufficient payment`);
            await blockCart.buyProduct(productIds[0], 1, { from: buyer1, value: web3.utils.toWei('0.5', 'ether') });
            assert.fail("Buyer was able to buy a product with insufficient payment. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
    });

    it("should handle excessive payment and refund correctly", async () => {
        const initialBalance = BigInt(await web3.eth.getBalance(buyer2));
        console.log(`Initial buyer1 balance: ${initialBalance}`);

        console.log(`Buyer ${buyer2} buying product ${productIds[0]} with excessive payment`);
        const excessiveAmount = web3.utils.toWei('10', 'ether');
        await blockCart.buyProduct(productIds[0], 1, { from: buyer2, value: excessiveAmount });
        const product = await blockCart.products(productIds[0]);
        assert.equal(product.inventory, 97, "Product inventory mismatch after purchase");

        const afterBalance = BigInt(await web3.eth.getBalance(buyer2));
        console.log(`After buyer1 balance: ${afterBalance}`);
    });

    it("should withdraw funds to the owner and verify the balance increase", async () => {
        const initialBalance = BigInt(await web3.eth.getBalance(owner));
        console.log(`Initial owner balance: ${initialBalance}`);
        await blockCart.withdrawFunds({ from: owner });
        const finalBalance = BigInt(await web3.eth.getBalance(owner));
        console.log(`Final owner balance: ${finalBalance}`);
        assert(finalBalance > initialBalance, "Owner balance did not increase after withdrawal");
    });

    it("should give total amount paid by buyer", async () => {
        let amountPaid = await blockCart.getBuyerAmountPaid(buyer1, { from: buyer1 });
        console.log(`Total paid by ${buyer1}: ${amountPaid}`);
    });

    it("Kill the contract", async () => {
        // Initial balance of the owner
        const initialBalance = BigInt(await web3.eth.getBalance(owner));
        console.log(`Initial owner balance: ${initialBalance}`);
        // Do a buy operation
        console.log(`Buyer ${buyer1} buying product ${productIds[0]} with quantity 2`);
        await blockCart.buyProduct(productIds[0], 2, { from: buyer1, value: web3.utils.toWei('2', 'ether') });
        // Kill the contract
        await blockCart.kill({ from: owner });
        // Try to add a product after the contract is killed
        try {
            await blockCart.addProduct(productIds[2], productNames[2], inventory, productPrices[2], { from: owner });
            assert.fail("Product was added after the contract was killed. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
        // Try to buy a product after the contract is killed
        try {
            console.log(`Buyer ${buyer1} attempting to buy product ${productIds[0]} after contract is killed`);
            await blockCart.buyProduct(productIds[0], 1, { from: buyer1, value: web3.utils.toWei('1', 'ether') });
            assert.fail("Buyer was able to buy a product after the contract was killed. This should have failed.");
        } catch (error) {
            assert.include(error.message, "revert", "Expected 'revert' error not received");
        }
        // Balance after the contract is killed
        const finalBalance = BigInt(await web3.eth.getBalance(owner));
        console.log(`Final owner balance: ${finalBalance}`);
        assert(finalBalance > initialBalance, "Owner balance did not increase after withdrawal");
    });
});
