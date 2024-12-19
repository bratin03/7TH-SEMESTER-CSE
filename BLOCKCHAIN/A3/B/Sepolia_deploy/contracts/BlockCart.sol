// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19; // Solidity version

/**
 * Deployed on Sepolia Testnet
 * Contract Address: 0x9e985fc0bB7a907ad2AA710d7721dca0D7C464cb
 * Transaction Hash: 0x38d7eca2ecba5a6a76c8463dc5c099a28e3f564f866a72bdd5f9ec6b4b2f50ae
 */

/**
 * @title BlockCart with Ethereum Smart Contract
 * @author Bratin Mondal(21CS10016), Abir Roy(21CS30074)
 * @notice This contract is developed as a part of an assignment for the course CS61065: Theory and Applications of Blockchain (https://cse.iitkgp.ac.in/~sandipc/courses/cs61065/cs61065.html) at IIT Kharagpur.
 * @dev This contract is developed to simulate a simple e-commerce platform where the owner can add and update products, withdraw funds, and deactivate the contract. Buyers can register, buy products, and check their total amount paid.
 */

/**
 * @title BlockCart
 * @dev A smart contract for managing products and buyers in an e-commerce system.
 */
contract BlockCart {
    // State variables
    address public owner; /** Address of the contract owner */
    uint public numBuyers; /** Total number of registered buyers */
    uint public numProducts; /** Total number of products available */
    uint public number; /** Total number of orders placed */
    bool public isActive; /** Status of the contract (active or inactive) */

    /**
     * @dev Structure to hold product details
     * @param ID Unique identifier for the product
     * @param name Name of the product
     * @param inventory Number of units available
     * @param price Price of the product in wei
     */
    struct Product {
        uint ID;
        string name;
        uint inventory;
        uint price;
    }

    /**
     * @dev Structure to hold buyer details
     * @param name Name of the buyer
     * @param email Email address of the buyer
     * @param mailingAddress Mailing address of the buyer
     * @param totalOrders Total number of orders placed by the buyer
     * @param isActive Status of the buyer (active or inactive)
     * @param amountPaid Total amount paid by the buyer
     */
    struct Buyer {
        string name;
        string email;
        string mailingAddress;
        uint totalOrders;
        bool isActive;
        uint amountPaid;
    }

    /**
     * @dev Structure to hold order details
     * @param orderID Unique identifier for the order
     * @param productId ID of the product being ordered
     * @param quantity Quantity of the product ordered
     * @param buyer Address of the buyer placing the order
     */
    struct Order {
        uint orderID;
        uint productId;
        uint quantity;
        address buyer;
    }

    // Mappings to store products, buyers, and orders
    mapping(uint => Product) public products; // Mapping of product ID to product details
    mapping(address => Buyer) public buyers; // Mapping of buyer address to buyer details
    mapping(uint => Order) public orders; // Mapping of order ID to order details

    // Events to log important contract activities
    /**
     * @dev Event to log the addition of a new product
     * @param _ID Unique ID of the product
     * @param _name Name of the product
     * @param _inventory Number of units available
     * @param _price Price of the product in wei
     */
    event NewProduct(
        uint indexed _ID,
        string _name,
        uint _inventory,
        uint _price
    );
    /**
     * @dev Event to log the registration of a new buyer
     * @param _name Name of the buyer
     * @param _email Email address of the buyer
     * @param _mailingAddress Mailing address of the buyer
     */
    event NewBuyer(string indexed _name, string _email, string _mailingAddress);
    /**
     * @dev Event to log the creation of a new order
     * @param _orderID Unique ID of the order
     * @param _ID ID of the product ordered
     * @param _quantity Quantity of the product ordered
     * @param _from Address of the buyer placing the order
     */
    event NewOrder(
        uint indexed _orderID,
        uint _ID,
        uint _quantity,
        address _from
    );

    /**
     * @dev Constructor to set the contract owner and initialize the contract
     */
    constructor() {
        owner = msg.sender; // Set the owner to the address deploying the contract
        numBuyers = 0;
        numProducts = 0;
        isActive = true; // Contract starts as active
    }

    /**
     * @dev Modifiers to restrict access to certain functions for the contract owner
     */
    modifier onlyOwner() {
        require(msg.sender == owner, "Only owner can call this function");
        _;
    }

    /**
     * @dev Modifier to restrict access to certain functions for registered buyers
     */
    modifier onlyRegisteredBuyer() {
        require(
            bytes(buyers[msg.sender].email).length > 0,
            "Buyer not registered"
        ); // Check if buyer is registered
        require(buyers[msg.sender].isActive, "Buyer is inactive");
        _; // Check if buyer is active
    }

    /**
     * @dev Modifier to check if the contract is active
     */
    modifier isActiveContract() {
        require(isActive, "Contract is not active");
        _; // Check if the contract is active
    }

    /**
     * @dev Function to add a new product
     * @param _ID Unique ID for the product
     * @param _name Name of the product
     * @param _inventory Number of units available
     * @param _price Price of the product in wei
     */
    function addProduct(
        uint _ID,
        string memory _name,
        uint _inventory,
        uint _price
    ) public onlyOwner isActiveContract {
        require(products[_ID].ID == 0, "Product with this ID already exists"); // Check for unique ID
        require(bytes(_name).length > 0, "Product name cannot be empty"); // Check for non-empty name

        // Add the new product
        products[_ID] = Product(_ID, _name, _inventory, _price);
        numProducts++; // Increment the total number of products
        emit NewProduct(_ID, _name, _inventory, _price); // Emit an event for the new product
    }

    /**
     * @dev Function to update an existing product
     * @param _ID Unique ID of the product to be updated
     * @param _name New name of the product
     * @param _inventory New inventory count
     * @param _price New price of the product in wei
     */
    function updateProduct(
        uint _ID,
        string memory _name,
        uint _inventory,
        uint _price
    ) public onlyOwner isActiveContract {
        require(products[_ID].ID == _ID, "Product not found"); // Check if product exists
        require(bytes(_name).length > 0, "Product name cannot be empty"); // Check for non-empty name

        // Update the product details
        products[_ID].name = _name;
        products[_ID].inventory = _inventory;
        products[_ID].price = _price;
    }

    /**
     * @dev Function to register a new buyer
     * @param _name Name of the buyer
     * @param _email Email address of the buyer
     * @param _mailingAddress Mailing address of the buyer
     */
    function registerBuyer(
        string memory _name,
        string memory _email,
        string memory _mailingAddress
    ) public isActiveContract {
        require(
            bytes(buyers[msg.sender].email).length == 0,
            "Buyer already registered"
        ); // Check if buyer is already registered

        buyers[msg.sender] = Buyer(_name, _email, _mailingAddress, 0, true, 0); // Register the new buyer
        numBuyers++; // Increment the total number of buyers
        emit NewBuyer(_name, _email, _mailingAddress); // Emit an event for the new buyer
    }

    /**
     * @dev Function to purchase a product
     * @param _ID ID of the product to be purchased
     * @param _quantity Quantity of the product to be purchased
     * @return newOrderID Unique ID of the newly created order
     */
    function buyProduct(
        uint _ID,
        uint _quantity
    )
        public
        payable
        onlyRegisteredBuyer
        isActiveContract
        returns (uint newOrderID)
    {
        Product memory product = products[_ID];
        require(product.ID == _ID, "Product not found"); // Check if product exists
        require(_quantity > 0, "Invalid quantity"); // Check for valid quantity
        require(_quantity <= product.inventory, "Insufficient inventory"); // Check for sufficient inventory

        uint totalAmount = _quantity * product.price; // Calculate the total amount to be paid
        require(msg.value >= totalAmount, "Insufficient payment"); // Check for sufficient payment

        // Update product inventory
        products[_ID].inventory -= _quantity;

        // Create a new order
        newOrderID = uint(
            keccak256(
                abi.encodePacked(msg.sender, _ID, _quantity, block.timestamp)
            )
        );
        orders[newOrderID] = Order(newOrderID, _ID, _quantity, msg.sender); // Add the new order

        // Update the buyer's total amount paid and total orders
        buyers[msg.sender].amountPaid += totalAmount; // Update the total amount paid by the buyer
        buyers[msg.sender].totalOrders++; // Increment the total orders placed by the buyer
        number++; // Increment the total number of orders

        // Refund excess payment if any
        if (msg.value > totalAmount) {
            payable(msg.sender).transfer(msg.value - totalAmount);
        }

        emit NewOrder(newOrderID, _ID, _quantity, msg.sender); // Emit an event for the new order
    }

    /**
     * @dev Function for the owner to withdraw contract balance
     */
    function withdrawFunds() public onlyOwner {
        payable(owner).transfer(address(this).balance); // Transfer the contract balance to the owner
    }

    /**
     * @dev Function to get the total amount paid by a specific buyer
     * @param buyer Address of the buyer
     * @return Total amount paid by the buyer
     */
    function getBuyerAmountPaid(address buyer) public view returns (uint) {
        return buyers[buyer].amountPaid;
    }

    /**
     * @dev Function to deactivate the contract
     * @notice Instead of using selfdestruct, the contract is deactivated
     */
    function kill() public onlyOwner {
        isActive = false; // Deactivate the contract
        withdrawFunds(); // Withdraw the contract balance
    }
}
