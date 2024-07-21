/*
    * Author: Bratin Mondal
    * Roll No: 21CS10016

    * Deparment of Computer Science and Engineering
    * Indian Institue of Technology, Kharagpur
*/

// Necessary header files
#include <iostream>
#include <vector>

/**
 * @brief Class to generate prime numbers using Sieve of Eratosthenes
 */
class Sieve
{
public:
    /**
     * @brief Construct a new Sieve object
     *
     * @param n Upper bound for prime numbers
     */
    Sieve(uint64_t n)
    {
        n_ = n;                   // Upper bound
        prime_ = new bool[n + 1]; // Boolean array to store prime numbers status
        // Sieve of Eratosthenes
        for (uint64_t i = 0; i <= n; i++)
        {
            prime_[i] = true; // Initially all numbers are prime
        }
        prime_[0] = prime_[1] = false; // 0 and 1 are not prime

        // Store prime numbers
        for (uint64_t i = 2; i * i <= n; i++)
        {
            if (prime_[i]) // If i is prime
            {
                primes_.push_back(i);                    // Store prime number
                for (uint64_t j = i * i; j <= n; j += i) // Mark all multiples of i as non-prime
                {
                    prime_[j] = false;
                }
            }
        }
    }

    /**
     * @brief Destroy the Sieve object
     */
    ~Sieve()
    {
        delete[] prime_; // Free memory
    }

    /**
     * @brief Check if a number is prime
     *
     * @param n Number to check
     * @return true if n is prime
     * @return false if n is not prime
     */
    bool is_prime(uint64_t n) const
    {
        if (n > n_)
            return false; // Out of range check
        return prime_[n];
    }

    /**
     * @brief Get a random prime number
     *
     * @return uint64_t Random prime number
     */
    uint64_t get_random() const
    {
        return primes_[rand() % primes_.size()];
    }

private:
    uint64_t n_;                   /**< Upper bound for prime numbers */
    bool *prime_;                  /**< Boolean array to store prime numbers status */
    std::vector<uint64_t> primes_; /**< Vector to store prime numbers */
};

/**
 * @brief Class to generate RSA key pair
 */
class RSA
{
public:
    /**
     * @brief Construct a new RSA object
     *
     * @param p Prime number
     * @param q Prime number
     */
    RSA(uint64_t p, uint64_t q) : p_(p), q_(q)
    {
        generate_key_pair();
    }

    /**
     * @brief Destroy the RSA object
     */
    ~RSA() {}

    /**
     * @brief Get public key
     *
     * @return std::pair<uint64_t, uint64_t> Public key
     */
    std::pair<uint64_t, uint64_t> get_public_key() const
    {
        return public_key_;
    }

    /**
     * @brief Get private key
     *
     * @return std::pair<uint64_t, uint64_t> Private key
     */
    std::pair<uint64_t, uint64_t> get_private_key() const
    {
        return private_key_;
    }

private:
    uint64_t p_, q_;                            /**< Prime numbers */
    std::pair<uint64_t, uint64_t> public_key_;  /**< Public key */
    std::pair<uint64_t, uint64_t> private_key_; /**< Private key */

    /**
     * @brief Compute greatest common divisor
     *
     * @param a Number
     * @param b Number
     * @return uint64_t GCD of a and b
     */
    uint64_t gcd(uint64_t a, uint64_t b) const
    {
        while (b != 0)
        {
            uint64_t temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    /**
     * @brief Compute modular inverse
     *
     * @param e Number
     * @param phi Number
     * @return uint64_t Modular inverse of e and phi
     */
    uint64_t mod_inverse(uint64_t e, uint64_t phi) const
    {

        int64_t t = 0, new_t = 1;
        uint64_t r = phi, new_r = e;
        while (new_r != 0)
        {
            uint64_t quotient = r / new_r;
            int64_t temp_t = t;
            t = new_t;
            new_t = temp_t - quotient * new_t;
            uint64_t temp_r = r;
            r = new_r;
            new_r = temp_r - quotient * new_r;
        }
        if (r > 1)
            return -1; // No modular inverse
        if (t < 0)
            t += phi;
        return t;
    }

    /**
     * @brief Check if a number is prime
     *
     * @param n Number
     * @return true if n is prime
     * @return false if n is not prime
     */
    bool prime_checker(uint64_t n) const
    {
        if (n <= 1)
            return false;
        if (n <= 3)
            return true;
        if (n % 2 == 0 || n % 3 == 0)
            return false;
        for (uint64_t i = 5; i * i <= n; i += 6)
        {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        }
        return true;
    }

    /**
     * @brief Generate RSA key pair
     *
     * @return void
     */
    void generate_key_pair()
    {
        if (!prime_checker(p_) || !prime_checker(q_))
        {
            std::cerr << "p and q must be prime numbers." << std::endl;
            exit(1);
        }

        if (p_ == q_)
        {
            std::cerr << "p and q must be distinct prime numbers." << std::endl;
            exit(1);
        }

        uint64_t n = p_ * q_;               // Modulus
        uint64_t phi = (p_ - 1) * (q_ - 1); // Euler's totient function

        Sieve sieve(std::min(1000000UL, phi));

        uint64_t e = rand() % phi;
        while (gcd(e, phi) != 1)
        {
            e = rand() % phi;
        }

        // Compute private exponent
        uint64_t d = mod_inverse(e, phi);

        public_key_ = {e, n};
        private_key_ = {d, n};
    }
};

/**
 * @brief Main function
 *
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 * @return 0 on successful execution
 */
int main(int argc, char const *argv[])
{
    uint64_t p, q;
    srand(time(0));
    if (argc == 3) // If p and q are provided
    {
        p = std::stoull(argv[1]);
        q = std::stoull(argv[2]);
    }
    else if (argc != 1) // If invalid number of arguments
    {
        std::cerr << "Usage: " << argv[0] << " <p> <q>" << std::endl;
        std::cerr << "p and q are prime numbers." << std::endl;
        std::cerr << "If p and q are not provided, random prime numbers will be generated." << std::endl;
        exit(1);
    }
    else // If p and q are not provided, generate random prime numbers
    {
        Sieve sieve(10000000);
        p = sieve.get_random();
        q = sieve.get_random();

        std::cout << "Random prime numbers generated: p = " << p << ", q = " << q << std::endl;
    }

    // Generate RSA key pair
    RSA rsa(p, q);

    auto public_key = rsa.get_public_key();
    auto private_key = rsa.get_private_key();

    std::cout << "Public key: (" << public_key.first << ", " << public_key.second << ")" << std::endl;
    std::cout << "Private key: (" << private_key.first << ", " << private_key.second << ")" << std::endl;

    return 0;
}
