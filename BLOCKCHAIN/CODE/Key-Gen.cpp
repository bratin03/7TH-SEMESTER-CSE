#include <iostream>
#include <utility>

// Class to handle RSA key pair generation
class RSA
{
public:
    RSA(int p, int q) : p_(p), q_(q)
    {
        generate_key_pair();
    }

    // Get public key
    std::pair<int, int> get_public_key() const
    {
        return {public_key_.first, public_key_.second};
    }

    // Get private key
    std::pair<int, int> get_private_key() const
    {
        return {private_key_.first, private_key_.second};
    }

private:
    int p_, q_;
    std::pair<int, int> public_key_;
    std::pair<int, int> private_key_;

    // Function to compute gcd
    int gcd(int a, int b) const
    {
        while (b != 0)
        {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    // Function to compute modular multiplicative inverse
    int mod_inverse(int e, int phi) const
    {
        int t = 0, new_t = 1;
        int r = phi, new_r = e;
        while (new_r != 0)
        {
            int quotient = r / new_r;
            int temp_t = t;
            t = new_t;
            new_t = temp_t - quotient * new_t;
            int temp_r = r;
            r = new_r;
            new_r = temp_r - quotient * new_r;
        }
        if (r > 1)
            return -1; // No modular inverse
        if (t < 0)
            t += phi;
        return t;
    }

    bool prime_checker(int n) const
    {
        if (n <= 1)
            return false;
        if (n <= 3)
            return true;
        if (n % 2 == 0 || n % 3 == 0)
            return false;
        for (int i = 5; i * i <= n; i += 6)
        {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        }
        return true;
    }

    // Generate RSA key pair
    void generate_key_pair()
    {
        if (!prime_checker(p_) || !prime_checker(q_))
        {
            std::cerr << "p and q must be prime numbers." << std::endl;
            exit(1);
        }
        int n = p_ * q_;
        int phi = (p_ - 1) * (q_ - 1);
        int e = 65537; // Common choice for e

        if (gcd(e, phi) != 1)
        {
            std::cerr << "e and phi(n) are not coprime, choose different e." << std::endl;
            exit(1);
        }

        int d = mod_inverse(e, phi);

        public_key_ = {e, n};
        private_key_ = {d, n};
    }
};

// Main function
int main()
{
    int p, q;
    std::cout << "Enter two prime numbers p and q: ";
    std::cin >> p >> q;

    RSA rsa(p, q);

    auto [e, n] = rsa.get_public_key();
    std::cout << "Public Key (e, n): (" << e << ", " << n << ")" << std::endl;

    auto [d, n2] = rsa.get_private_key();
    std::cout << "Private Key (d, n): (" << d << ", " << n2 << ")" << std::endl;

    return 0;
}
