/*
    * Author: Bratin Mondal
    * Roll No: 21CS10016

    * Deparment of Computer Science and Engineering
    * Indian Institue of Technology, Kharagpur
*/

// Necessary header files
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cassert>
#include <cstddef>

#define FILE_INPUT // Comment this line if you want to take input from standard input

#define PARALLEL_MINING // Comment this line if you want to mine blocks sequentially, otherwise blocks will be mined in parallel using multiple threads

#ifdef PARALLEL_MINING
#include <thread>
#include <atomic>
#endif

const std::string FILE_NAME = "input.txt"; // Change this to the input file name

/*
This SHA256 implementation is a modified version of the one found at https://github.com/System-Glitch/SHA256/blob/master/src/SHA256.cpp
*/

/**
 * @class SHA256
 * @brief Class to implement SHA256 hashing algorithm
 */
class SHA256
{
    // Public Methods
public:
    /**
     * @brief Constructs a new SHA256 object.
     *
     * Initializes the internal state and prepares the object for hashing.
     */
    SHA256()
    {
        reset(); // Reset the state
    }

    /**
     * @brief Destroys the SHA256 object.
     */
    ~SHA256()
    {
    }

    /**
     * @brief Hashes the given data.
     *
     * @param data The data to be hashed.
     * @return The hash of the data.
     */
    std::string hash(const std::string &data)
    {
        return update(data).final(); // Update the data and get the final hash
    }

    // Private Methods and Members
private:
    uint32_t m_state[8]; /**< The internal state of the SHA256 algorithm. */
    uint8_t m_data[64];  /**< The data block being processed. */
    uint32_t m_blocklen; /**< The length of the current data block. */
    uint64_t m_bitlen;   /**< The total length of the input data in bits. */

    static constexpr uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2}; /** < The constants used in the SHA256 algorithm. */

    /**
     * @brief Rotates the bits of a 32-bit unsigned integer to the right.
     *
     * @param x The 32-bit unsigned integer to be rotated.
     * @param n The number of bits to rotate.
     * @return The rotated value.
     */
    static uint32_t rotr(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    /**
     * @brief Chooses bits based on the majority rule.
     *
     * This function selects bits based on the majority rule.
     *
     * @param e The first input value.
     * @param f The second input value.
     * @param g The third input value.
     * @return The result of the majority function.
     */
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g)
    {
        return (e & f) ^ (~e & g);
    }

    /**
     * @brief Computes the majority value among three inputs.
     *
     * This function computes the majority value among three inputs.
     *
     * @param a The first input value.
     * @param b The second input value.
     * @param c The third input value.
     * @return The result of the majority function.
     */
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c)
    {
        return (a & b) ^ (a & c) ^ (b & c);
    }

    /**
     * @brief Computes the first SHA256-specific bitwise operation.
     *
     * @param x The input value.
     * @return The result of the bitwise operation.
     */
    static uint32_t sig0(uint32_t x)
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    /**
     * @brief Computes the second SHA256-specific bitwise operation.
     *
     * @param x The input value.
     * @return The result of the bitwise operation.
     */
    static uint32_t sig1(uint32_t x)
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    /**
     * @brief Transforms the current block.
     *
     * Processes the current 512-bit block of data and updates the internal state.
     * @return void
     */
    void transform()
    {
        uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
        uint32_t state[8];

        // Initialize the first 16 words in the message schedule array (m) from the input data
        for (uint32_t i = 0, j = 0; i < 16; ++i, j += 4)
        {
            m[i] = (m_data[j] << 24) | (m_data[j + 1] << 16) | (m_data[j + 2] << 8) | (m_data[j + 3]);
        }

        // Extend the first 16 words into the remaining 48 words in the message schedule array
        for (uint32_t k = 16; k < 64; ++k)
        {
            m[k] = sig1(m[k - 2]) + m[k - 7] + sig0(m[k - 15]) + m[k - 16];
        }

        // Initialize working variables with the current hash value
        for (uint32_t i = 0; i < 8; ++i)
        {
            state[i] = m_state[i];
        }

        // Perform the main hash computation on the message schedule array
        for (uint32_t i = 0; i < 64; ++i)
        {
            // Calculate the majority function on state[0], state[1], state[2]
            maj = majority(state[0], state[1], state[2]);

            // Calculate the XOR of state[0] with its right-rotated versions
            xorA = rotr(state[0], 2) ^ rotr(state[0], 13) ^ rotr(state[0], 22);

            // Calculate the choose function on state[4], state[5], state[6]
            ch = choose(state[4], state[5], state[6]);

            // Calculate the XOR of state[4] with its right-rotated versions
            xorE = rotr(state[4], 6) ^ rotr(state[4], 11) ^ rotr(state[4], 25);

            // Compute the sum and update the working variables
            sum = m[i] + k[i] + state[7] + ch + xorE;
            newA = xorA + maj;
            newE = state[3] + sum;

            // Update the working variables for the next round
            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = newE;
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = sum + newA;
        }

        // Add the compressed chunk to the current hash value
        for (uint32_t i = 0; i < 8; ++i)
        {
            m_state[i] += state[i];
        }
    }

    /**
     * @brief Updates the internal state with a chunk of data.
     *
     * This method processes the input data chunk by chunk, updating the internal state as needed.
     *
     * @param data The data chunk to be processed.
     * @param length The length of the data chunk in bytes.
     * @return A reference to the current SHA256 object.
     */
    SHA256 &update(const unsigned char *data, ssize_t length)
    {
        // Process each byte in the input data
        for (ssize_t i = 0; i < length; ++i)
        {
            // Append the current byte to the data block being accumulated
            m_data[m_blocklen++] = data[i];

            // If the block is full (64 bytes), process it
            if (m_blocklen == 64)
            {
                transform();     // Transform the current block into the hash state
                m_bitlen += 512; // Increment the bit length by the size of the block (512 bits)
                m_blocklen = 0;  // Reset the block length to start a new block
            }
        }
        return *this; // Return reference to the current object for chaining
    }

    /**
     * @brief Updates the internal state with a string.
     *
     * This method is a convenience overload that processes string data by converting it to raw bytes.
     *
     * @param data The string data to be processed.
     * @return A reference to the current SHA256 object.
     */
    SHA256 &update(const std::string &data)
    {
        // Convert the string to a byte array and update the hash
        return update(reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
    }

    /**
     * @brief Finalizes the hashing process and returns the hash value.
     *
     * This method pads the remaining data, processes any remaining blocks, and returns the final SHA256 hash value.
     *
     * @return The final SHA256 hash value in hexadecimal format.
     */
    std::string final()
    {
        uint32_t i = m_blocklen;

        // Pad the current block to 56 bytes (448 bits) if there's enough space
        if (m_blocklen < 56)
        {
            m_data[i++] = 0x80; // Append the bit '1' to the message (10000000 in binary)
            while (i < 56)
                m_data[i++] = 0x00; // Append zeros until the block is 56 bytes long
        }
        else
        {
            // If the current block doesn't have enough space, pad it and transform
            m_data[i++] = 0x80; // Append the bit '1'
            while (i < 64)
                m_data[i++] = 0x00; // Append zeros to fill the block
            transform();            // Process the current block
            memset(m_data, 0, 56);  // Prepare a new block by setting the first 56 bytes to zero
        }

        // Append the length of the original message (in bits) as a 64-bit big-endian integer
        m_bitlen += m_blocklen * 8; // Update the total bit length
        m_data[63] = m_bitlen;
        m_data[62] = m_bitlen >> 8;
        m_data[61] = m_bitlen >> 16;
        m_data[60] = m_bitlen >> 24;
        m_data[59] = m_bitlen >> 32;
        m_data[58] = m_bitlen >> 40;
        m_data[57] = m_bitlen >> 48;
        m_data[56] = m_bitlen >> 56;
        transform(); // Process the final block

        // Convert the resulting hash state to a hexadecimal string
        std::ostringstream result;
        for (uint32_t i = 0; i < 8; ++i)
        {
            result << std::hex << std::setw(8) << std::setfill('0') << m_state[i];
        }

        reset();             // Reset the hash state for a new calculation
        return result.str(); // Return the final hash value
    }

    /**
     * @brief Resets the internal state.
     *
     * Resets the internal state of the SHA256 object, allowing it to be reused for another hash computation.
     */
    void reset()
    {
        // Initialize the hash state with the SHA256 initial constants
        m_state[0] = 0x6a09e667;
        m_state[1] = 0xbb67ae85;
        m_state[2] = 0x3c6ef372;
        m_state[3] = 0xa54ff53a;
        m_state[4] = 0x510e527f;
        m_state[5] = 0x9b05688c;
        m_state[6] = 0x1f83d9ab;
        m_state[7] = 0x5be0cd19;

        m_blocklen = 0; // Reset the block length
        m_bitlen = 0;   // Reset the total bit length
    }
};

constexpr uint32_t SHA256::k[64]; // Define the static member variable

/**
 * @class MerkleTree
 * @brief Class to implement a Merkle Tree using the SHA256 hashing algorithm
 */
class MerkleTree
{
public:
    /**
     * @brief Constructor
     */
    MerkleTree()
    {
        m_sha256 = std::make_unique<SHA256>(); // Create a unique pointer to a SHA256 object
    }

    /**
     * @brief Destructor
     */
    ~MerkleTree()
    {
    }

    /**
     * @brief Generate the Merkle root from a list of transactions
     * @param transactions A vector of strings representing the transactions
     * @return A vector of strings representing the child nodes of the Merkle root
     */
    std::vector<std::string> hash(const std::vector<std::string> &transactions)
    {
        std::vector<std::string> hashes;

        // Hash each transaction individually and store the hashes
        for (const auto &transaction : transactions)
        {
            hashes.push_back(m_sha256->hash(transaction)); // Hash each transaction and add it to the list
        }

        // Continue hashing until the list is reduced to 1 or 2 hashes
        while (hashes.size() > 2)
        {
            // If the number of hashes is odd, duplicate the last hash to make it even
            if (hashes.size() % 2 != 0)
            {
                hashes.push_back(hashes.back()); // Duplicate the last hash
            }

            std::vector<std::string> new_hashes;

            // Hash pairs of hashes together and store the new hashes
            for (unsigned long i = 0; i < hashes.size(); i += 2)
            {
                new_hashes.push_back(m_sha256->hash(hashes[i] + hashes[i + 1])); // Concatenate and hash pairs (No space between hashes)
            }

            hashes = new_hashes; // Replace the old hashes with the new set of hashes
        }

        return hashes; // Return the child nodes of the Merkle root
    }

private:
    std::unique_ptr<SHA256> m_sha256; // Unique pointer to the SHA256 object used for hashing
};

/**
 * @class Block
 * @brief Class representing a block in a blockchain, capable of mining a valid block hash.
 */
class Block
{
public:
    /**
     * @brief Default Constructor
     */
    Block()
    {
        m_sha256 = std::make_unique<SHA256>();          // Initialize SHA256 hasher
        m_merkle_tree = std::make_unique<MerkleTree>(); // Initialize Merkle Tree for transactions
    }

    /**
     * @brief Parameterized Constructor
     *
     * @param transactions A vector of strings representing the transactions to be included in the block.
     * @param z The number of leading zeros required in the valid block hash.
     * @param d The divisor used to validate the sum of ASCII values of the block hash.
     */
    Block(const std::vector<std::string> &transactions, uint64_t z, uint64_t d)
        : m_transactions(transactions), z(z), d(d)
    {
        m_sha256 = std::make_unique<SHA256>();          // Initialize SHA256 hasher
        m_merkle_tree = std::make_unique<MerkleTree>(); // Initialize Merkle Tree for transactions
    }

    /**
     * @brief Destructor
     */
    ~Block()
    {
    }

    /**
     * @brief Mines the block by finding a valid nonce
     *
     * @param transactions A vector of strings representing the transactions to be included in the block.
     * @return A pair consisting of the nonce and the valid hash found during the mining process.
     */
    std::pair<uint64_t, std::string> mine(const std::vector<std::string> &transactions)
    {
        auto merkle_root = m_merkle_tree->hash(transactions); // Generate the Merkle root from transactions
        std::string merkle_root_str;

        // Concatenate the Merkle root hashes into a single string
        if (merkle_root.size() == 1)
        {
            merkle_root_str = merkle_root[0] + merkle_root[0]; // Duplicate the single Merkle root hash
        }
        else if (merkle_root.size() == 2)
        {
            merkle_root_str = merkle_root[0] + merkle_root[1]; // Concatenate the two Merkle root hashes
        }
        else
        {
            std::cout << "Error: Invalid Merkle Root" << std::endl;
            return {0, ""}; // Return error if Merkle root is invalid
        }

        std::string hash;
        uint64_t nonce = 0;

        // Mining loop: find a nonce such that the hash is valid
        while (true)
        {
            hash = m_sha256->hash(merkle_root_str + std::to_string(nonce)); // Hash the Merkle root and nonce
            if (is_valid(hash))
            {
                return {nonce, hash}; // Return the valid nonce and hash
            }
            else
            {
                // Check for overflow and increment nonce
                if (nonce == UINT64_MAX)
                {
                    return {0, ""}; // Return error if nonce overflows
                }
                ++nonce; // Increment nonce and try again
            }
        }
        return {0, ""}; // Return error if mining fails
    }
#ifdef PARALLEL_MINING
    /**
     * @brief Mines the block in parallel using multiple threads
     *
     * @param transactions A vector of strings representing the transactions to be included in the block.
     * @param num_threads The number of threads to be used for mining.
     * @return A pair consisting of the nonce and the valid hash found during the mining process.
     */
    std::pair<uint64_t, std::string> mine_parallel(const std::vector<std::string> &transactions, int num_threads = 8)
    {
        auto merkle_root = m_merkle_tree->hash(transactions); // Generate the Merkle root from transactions
        std::string merkle_root_str;

        // Concatenate the Merkle root hashes into a single string
        if (merkle_root.size() == 1)
        {
            merkle_root_str = merkle_root[0] + merkle_root[0]; // Duplicate the single Merkle root hash
        }
        else if (merkle_root.size() == 2)
        {
            merkle_root_str = merkle_root[0] + merkle_root[1]; // Concatenate the two Merkle root hashes
        }
        else
        {
            std::cout << "Error: Invalid Merkle Root" << std::endl;
            return {0, ""}; // Return error if Merkle root is invalid
        }

        std::atomic<uint64_t> nonce(0);
        std::atomic<bool> found(false);
        std::string hash;
        std::pair<uint64_t, std::string> result = {0, ""};

        // Function to be executed by each thread
        auto worker = [&](uint64_t start_nonce)
        {
            uint64_t local_nonce = start_nonce;

            // create a new sha256 object for each thread
            std::unique_ptr<SHA256> sha256 = std::make_unique<SHA256>();

            while (!found)
            {
                std::string local_hash = sha256->hash(merkle_root_str + std::to_string(local_nonce)); // Hash the Merkle root and nonce
                if (is_valid(local_hash))
                {
                    if (!found.exchange(true))
                    {
                        result = {local_nonce, local_hash}; // Store the result when a valid hash is found
                    }
                    return;
                }
                // Check for overflow and increment nonce
                if (local_nonce >= UINT64_MAX - num_threads)
                {
                    return; // Return if nonce overflows
                }
                local_nonce += num_threads;
            }
        };

        // Create and start threads
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(worker, i);
        }

        // Wait for all threads to finish
        for (auto &t : threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        return result; // Return the found nonce and hash
    }
#endif

private:
    std::unique_ptr<SHA256> m_sha256;          // Unique pointer to the SHA256 hasher
    std::unique_ptr<MerkleTree> m_merkle_tree; // Unique pointer to the Merkle Tree
    std::vector<std::string> m_transactions;   // List of transactions included in the block
    uint64_t z;                                // Number of leading zeros required in the valid hash
    uint64_t d;                                // Divisor for sum of ASCII values in the valid hash

    /**
     * @brief Validates the block hash based on the criteria
     *
     * @param hash A string representing the hash to be validated.
     * @return True if the hash is valid, false otherwise.
     */
    bool is_valid(const std::string &hash)
    {
        // Check if the hash has z leading zeros
        for (uint64_t i = 0; i < z; ++i)
        {
            if (hash[i] != '0')
            {
                return false; // Hash does not have the required leading zeros
            }
        }

        // Calculate the sum of ASCII values of the hash characters
        uint64_t sum = 0;
        for (const auto &c : hash)
        {
            sum += c;
        }

        // Check if the sum is divisible by d
        return sum % d == 0;
    }
};

/**
 * @brief Main function to read input and mine blocks
 *
 * @return 0 on successful execution
 */
int main()
{
    // If FILE_INPUT is defined, read input from the input file
#ifdef FILE_INPUT
    FILE *file = freopen(FILE_NAME.c_str(), "r", stdin);
    assert(file != nullptr);
#endif
    uint64_t b, t, s, z, d; // Input parameters
    s = 20;                 // Length of transactions is fixed at 20 characters

// If FILE_INPUT is defined, read input from the input file one block at a time, process it, and print the results and then read the next block
#ifdef FILE_INPUT
    std::cin >> b; // Read the number of blocks
    for (uint64_t i = 0; i < b; ++i)
    {
        std::cin >> t;                            // Read the number of transactions
        std::vector<std::string> transactions(t); // Vector to store transactions
        for (uint64_t j = 0; j < t; ++j)
        {
            std::cin >> transactions[j]; // Read the transactions
            assert(transactions[j].length() == s);
        }
        std::cin >> z >> d;              // Read the difficulty parameters
        Block block(transactions, z, d); // Create a Block object
#ifdef PARALLEL_MINING
        auto mining_result = block.mine_parallel(transactions); // Mine the block
#else
        auto mining_result = block.mine(transactions); // Mine the block
#endif
        auto nonce = mining_result.first;
        auto hash = mining_result.second;
        // Print the results
        if (nonce == 0 && hash == "")
        {
            std::cout << "Error: Mining failed" << std::endl;
        }
        else
        {
            std::cout << "Valie Nonce: " << nonce << std::endl;
            std::cout << "Hash: " << hash << std::endl;
        }
    }
#else
    // If FILE_INPUT is not defined, read input from standard input all at once, process it, and print the results
    std::cin >> b;                                                                                               // Read the number of blocks
    std::vector<std::pair<std::pair<uint64_t, std::vector<std::string>>, std::pair<uint64_t, uint64_t>>> blocks; // Vector to store block parameters
    for (uint64_t i = 0; i < b; ++i)
    {
        std::cin >> t;                            // Read the number of transactions
        std::vector<std::string> transactions(t); // Vector to store transactions
        for (uint64_t j = 0; j < t; ++j)
        {
            std::cin >> transactions[j]; // Read the transactions
            assert(transactions[j].length() == s);
        }
        std::cin >> z >> d;                            // Read the difficulty parameters
        blocks.push_back({{t, transactions}, {z, d}}); // Add the block parameters to the vector
    }

    // Process each block and print the results
    for (const auto &block : blocks)
    {
        auto transactions = block.first.second;
        auto z = block.second.first;
        auto d = block.second.second;
        Block block_obj(transactions, z, d); // Create a Block object
#ifdef PARALLEL_MINING
        auto mining_result = block_obj.mine_parallel(transactions); // Mine the block
#else
        auto mining_result = block_obj.mine(transactions); // Mine the block
#endif
        auto nonce = mining_result.first;
        auto hash = mining_result.second;
        // Print the results
        if (nonce == 0 && hash == "")
        {
            std::cout << "Error: Mining failed" << std::endl;
        }
        else
        {
            std::cout << "Valie Nonce: " << nonce << std::endl;
            std::cout << "Hash: " << hash << std::endl;
        }
    }
#endif
    return 0; // Return 0 on successful execution
}