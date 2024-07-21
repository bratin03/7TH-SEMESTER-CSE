/*
    * Author: Bratin Mondal
    * Roll No: 21CS10016

    * Deparment of Computer Science and Engineering
    * Indian Institue of Technology, Kharagpur
*/

/*
Assuming that the public and private keys are given in the following format:
e n
d n

Where e and d are the public and private exponents, respectively, and n is the modulus value.
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <cstring>
#include <iomanip>

const std::string input_file = "input.txt"; // Input file name // change this to the input file name

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

private:
    uint32_t m_state[8]; /**< The internal state of the SHA256 algorithm. */
    uint8_t m_data[64];  /**< The data block being processed. */
    uint32_t m_blocklen; /**< The length of the current data block. */
    uint64_t m_bitlen;   /**< The total length of the input data in bits. */

    /** < The round constants. */
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
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

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
     *
     * @param transactions A vector of strings representing the transactions
     * @return The Merkle root hash
     */
    std::string hash(const std::vector<std::string> &transactions)
    {
        std::vector<std::string> hashes;
        if (transactions.empty())
        {
            return "";
        }

        // Hash each transaction individually and store the hashes
        for (const auto &transaction : transactions)
        {
            hashes.push_back(m_sha256->hash(transaction)); // Hash each transaction and add it to the list
        }

        // Continue hashing until the list is reduced to 1 hash (the Merkle root)
        while (hashes.size() > 1)
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
        return hashes[0]; // Return the final hash as the Merkle root
    }

private:
    std::unique_ptr<SHA256> m_sha256; // Unique pointer to the SHA256 object used for hashing
};

/**
 * @class RSA_Key
 * @brief Class to implement RSA encryption and decryption
 *
 * This class provides methods to encode and decode data using RSA encryption.
 */
class RSA_Key
{

public:
    /**
     * @brief Constructor
     */
    RSA_Key()
    {
        // Indicate uninitialized key values
        n = 0;
        e = 0;
    }

    /**
     * @brief Constructor with initialization
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     */
    RSA_Key(uint64_t n, uint64_t e)
    {
        this->n = n;
        this->e = e;
    }

    /**
     * @brief Destructor
     */
    ~RSA_Key()
    {
    }

    /**
     * @brief Set the key values for the RSA key
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     * @return void
     */
    void set_key(uint64_t n, uint64_t e)
    {
        this->n = n;
        this->e = e;
    }

    /**
     * @brief Encode data using RSA encryption
     *
     * @param data The data to be encoded
     * @return The encoded data
     */
    std::string encode(const std::string &data)
    {
        // Check if the key values are initialized
        if (n == 0 || e == 0)
        {
            return data; // Return the data as is if the key values are not initialized
        }
        std::string encoded_data = "";
        for (char c : data)
        {
            encoded_data += std::to_string(mod_exp(c)) + " "; // Encode each character and add it to the string with a space
        }
        return encoded_data;
    }

    /**
     * @brief Decode data using RSA decryption
     *
     * @param data The data to be decoded
     * @return The decoded data
     */
    std::string decode(const std::string &data)
    {
        // Check if the key values are initialized
        if (n == 0 || e == 0)
        {
            return data; // Return the data as is if the key values are not initialized
        }
        std::string decoded_data = "";
        std::stringstream ss(data);
        std::string token;
        // Split the data string by spaces and decode each token
        while (getline(ss, token, ' '))
        {
            decoded_data += (char)mod_exp(stoull(token));
        }
        return decoded_data;
    }

protected:
    uint64_t n; /**< The modulus value for the RSA key */
    uint64_t e; /**< The public exponent value for the RSA key */

    /**
     * @brief Compute the modular exponentiation of a number
     *
     * @param base The base value
     * @return The result of the modular exponentiation
     */
    unsigned long long mod_exp(uint64_t base)
    {
        unsigned long long result = 1;
        auto e_copy = e;
        while (e_copy > 0)
        {
            if (e_copy % 2 == 1)
            {
                result = (result * base) % n;
            }
            base = (base * base) % n;
            e_copy /= 2;
        }
        return result;
    }
};

/**
 * @class RSA_Key_Pair
 * @brief Class to implement RSA encryption and decryption using a key pair
 *
 * This class provides methods to encode and decode data using RSA encryption with a key pair.
 */
class RSA_Key_Pair
{
public:
    /**
     * @brief Constructor
     */
    RSA_Key_Pair() {}

    /**
     * @brief Constructor with initialization
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     * @param d The private exponent value for the RSA key
     */
    RSA_Key_Pair(uint64_t n, uint64_t e, uint64_t d) : public_key(n, e), private_key(n, d) {}

    /**
     * @brief Destructor
     */
    ~RSA_Key_Pair() {}

    /**
     * @brief Set the key pair values for the RSA key
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     * @param d The private exponent value for the RSA key
     * @return void
     */

    void set_key_pair(uint64_t n, uint64_t e, uint64_t d)
    {
        public_key.set_key(n, e);
        private_key.set_key(n, d);
    }

    /**
     * @brief Encode data using public key RSA encryption
     *
     * @param data The data to be encoded
     * @return The encoded data
     */
    std::string encrypt(const std::string &data)
    {
        return public_key.encode(data);
    }

    /**
     * @brief Decode data using private key RSA decryption
     *
     * @param data The data to be decoded
     * @return The decoded data
     */
    std::string decrypt(const std::string &data)
    {
        return private_key.decode(data);
    }

    /**
     * @brief Sign data using private key RSA encryption
     *
     * @param data The data to be signed
     * @return The signed data
     */
    std::string sign(const std::string &data)
    {
        return private_key.encode(data);
    }

    /**
     * @brief Verify data using public key RSA decryption
     *
     * @param data The data to be verified
     * @return The verified data
     */
    std::string verify(const std::string &data)
    {
        return public_key.decode(data);
    }

private:
    RSA_Key public_key;  /**< The public key for RSA encryption */
    RSA_Key private_key; /**< The private key for RSA decryption */
};

/**
 * @class Block
 * @brief Class to implement a block in a blockchain
 */
class Block
{
public:
    /**
     * @brief Constructor with initialization
     */
    Block(const std::string &previous_hash_signature, const std::string &previous_hash, const std::vector<std::string> &transactions, bool first_block = false, const std::string &block_version = "02000000")
        : previous_hash_RSA_signature(previous_hash_signature), previous_hash(previous_hash), transactions(transactions), first_block(first_block), block_version(block_version), nonce(generate_nonce())
    {
        initialize_merkle_root(); // Initialize the Merkle root
    }

    /**
     * @brief Destructor
     */
    ~Block()
    {
    }

    /**
     * @brief Set the public key of the previous block
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     * @return void
     */
    void set_prev_block_public_key(uint64_t n, uint64_t e)
    {
        prev_block_public_key.set_key(n, e);
    }

    /**
     * @brief Set the key pair for the current block
     *
     * @param n The modulus value for the RSA key
     * @param e The public exponent value for the RSA key
     * @param d The private exponent value for the RSA key
     * @return void
     */
    void set_block_key_pair(uint64_t n, uint64_t e, uint64_t d)
    {
        block_key_pair.set_key_pair(n, e, d);
    }

    /**
     * @brief Set the block hash signature
     *
     * @return void
     */
    void set_block_hash_signature()
    {
        compute_block_hash();
        sign_block_hash();
    }

    /**
     * @brief Get the block hash signature
     *
     * @return The block hash signature
     */
    std::string get_block_hash_signature() const
    {
        return block_hash_signature;
    }

    /**
     * @brief Get the block hash RSA signature
     *
     * @return The block hash RSA signature
     */
    std::string get_block_hash_RSA_signature() const
    {
        return block_hash_RSA_signature;
    }

    /**
     * @brief Verify the block (previous hash signature and current hash signature)
     *
     * @return True if the block is valid, false otherwise
     */
    bool verify_block()
    {
        return verify_previous_block() && verify_current_hash();
    }

    // Private methods
private:
    std::string previous_hash_RSA_signature; /**< The RSA signature of the previous block hash */
    std::string previous_hash;               /**< The hash of the previous block */
    std::string merkle_root;                 /**< The Merkle root hash of the transactions in the block */
    std::vector<std::string> transactions;   /**< The list of transactions in the block */
    bool first_block;                        /**< Flag to indicate if this is the first block in the blockchain */
    std::string block_version;               /**< The version of the block */
    uint64_t nonce;                          /**< The nonce value for the block */
    std::string block_hash_signature;        /**< The signature of the block hash */
    std::string block_hash_RSA_signature;    /**< The RSA signature of the block hash */
    RSA_Key prev_block_public_key;           /**< The public key of the previous block */
    RSA_Key_Pair block_key_pair;             /**< The key pair for the current block */

    /**
     * @brief Initialize the Merkle root
     *
     * @return void
     */
    void initialize_merkle_root()
    {
        MerkleTree merkle_tree;
        merkle_root = merkle_tree.hash(transactions);
    }

    /**
     * @brief Generate a nonce value
     *
     * @return The nonce value
     */
    uint64_t generate_nonce() const
    {
        return rand(); // Generate a random nonce value
    }

    /**
     * @brief Compute the block hash
     *
     * @return void
     */
    void compute_block_hash()
    {
        SHA256 sha256;
        std::string data = block_version + previous_hash + merkle_root + std::to_string(nonce);
        block_hash_signature = sha256.hash(data);
    }

    /**
     * @brief Sign the block hash
     *
     * @return void
     */
    void sign_block_hash()
    {
        block_hash_RSA_signature = block_key_pair.sign(block_hash_signature);
    }

    /**
     * @brief Verify the previous block hash
     *
     * @return True if the previous block hash is valid, false otherwise (special case for the first block, which is always valid)
     */
    bool verify_previous_block()
    {
        return first_block || previous_hash == prev_block_public_key.decode(previous_hash_RSA_signature);
    }

    /**
     * @brief Verify the current block hash
     *
     * @return True if the current block hash is valid, false otherwise
     */
    bool verify_current_hash()
    {
        return block_hash_signature == block_key_pair.verify(block_hash_RSA_signature);
    }
};

/**
 * @brief Main function
 *
 * @return 0
 */
int main()
{
    std::vector<std::string> coinbase_transactions = {"coinbase"}; // The coinbase transaction
    Block coinbase_block("", "", coinbase_transactions);           // Create the coinbase block
    coinbase_block.set_block_hash_signature();                     // Set the block hash signature for the coinbase block
    srand(time(0));                                                // Seed the random number generator

    // redircting input to input.txt
    auto file = freopen(input_file.c_str(), "r", stdin);
    if (file == nullptr)
    {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }

    // Read the number of blocks
    uint64_t B;
    std::cin >> B;

    uint64_t prev_public_key_n = 0;                                          // Initialize the public key of the previous block
    uint64_t prev_public_key_e = 0;                                          // Initialize the public key of the previous block
    std::string prev_block_hash = coinbase_block.get_block_hash_signature(); // Initialize the hash of the previous block
    std::string prev_block_hash_RSA_signature;                               // Coin base block has no key pair for signing

    bool chain_broken = false; // Flag to indicate if the chain is broken

    for (uint64_t i = 0; i < B; i++)
    {
        uint64_t T; // Number of transactions in the block
        std::cin >> T;
        std::vector<std::string> transactions;
        for (uint64_t j = 0; j < T; j++)
        {
            std::string transaction;
            std::cin >> transaction;
            transactions.push_back(transaction);
        }

        bool first_block = (i == 0); // Flag to indicate if this is the first block

        Block block(prev_block_hash_RSA_signature, prev_block_hash, transactions, first_block); // Create the block

        uint64_t n, n2, e, d; // RSA key values
        std::cin >> e >> n;
        std::cin >> d >> n2;

        // Check if the public and private key exponents are the same
        if (n != n2)
        {
            std::cerr << "Public and private keys do not match for block " << i + 1 << std::endl;
            exit(1);
        }
        block.set_block_key_pair(n, e, d); // Set the key pair for the block

        // If this is not the first block, set the public key of the previous block
        if (!first_block)
        {
            block.set_prev_block_public_key(prev_public_key_n, prev_public_key_e);
        }

        block.set_block_hash_signature(); // Set the block hash signature

        // Update the previous block values
        prev_block_hash = block.get_block_hash_signature();
        prev_block_hash_RSA_signature = block.get_block_hash_RSA_signature();
        prev_public_key_n = n;
        prev_public_key_e = e;

        // Verify the block
        if (!chain_broken && block.verify_block()) // If the chain is not broken and the block is valid
        {
            std::cout << "Valid" << std::endl;
        }
        else // If the chain is broken or the block is invalid
        {
            std::cout << "Invalid" << std::endl;
            chain_broken = true; // Set the chain broken flag
        }
    }

    fclose(file); // Close the input file
    return 0;
}