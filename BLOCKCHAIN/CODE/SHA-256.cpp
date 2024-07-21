#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>

class SHA256
{
public:
    SHA256()
    {
        reset();
    }

    SHA256 &update(const unsigned char *data, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
        {
            m_data[m_blocklen++] = data[i];
            if (m_blocklen == 64)
            {
                transform();
                m_bitlen += 512;
                m_blocklen = 0;
            }
        }
        return *this;
    }

    SHA256 &update(const std::string &data)
    {
        return update(reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
    }

    std::string final()
    {
        uint32_t i = m_blocklen;

        if (m_blocklen < 56)
        {
            m_data[i++] = 0x80;
            while (i < 56)
                m_data[i++] = 0x00;
        }
        else
        {
            m_data[i++] = 0x80;
            while (i < 64)
                m_data[i++] = 0x00;
            transform();
            memset(m_data, 0, 56);
        }

        m_bitlen += m_blocklen * 8;
        m_data[63] = m_bitlen;
        m_data[62] = m_bitlen >> 8;
        m_data[61] = m_bitlen >> 16;
        m_data[60] = m_bitlen >> 24;
        m_data[59] = m_bitlen >> 32;
        m_data[58] = m_bitlen >> 40;
        m_data[57] = m_bitlen >> 48;
        m_data[56] = m_bitlen >> 56;
        transform();

        std::ostringstream result;
        for (uint32_t i = 0; i < 8; ++i)
        {
            result << std::hex << std::setw(8) << std::setfill('0') << m_state[i];
        }
        reset();
        return result.str();
    }

private:
    uint32_t m_state[8];
    uint8_t m_data[64];
    uint32_t m_blocklen;
    uint64_t m_bitlen;

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

    static uint32_t rotr(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g)
    {
        return (e & f) ^ (~e & g);
    }

    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c)
    {
        return (a & b) ^ (a & c) ^ (b & c);
    }

    static uint32_t sig0(uint32_t x)
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static uint32_t sig1(uint32_t x)
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    void transform()
    {
        uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
        uint32_t state[8];

        for (uint32_t i = 0, j = 0; i < 16; ++i, j += 4)
        {
            m[i] = (m_data[j] << 24) | (m_data[j + 1] << 16) | (m_data[j + 2] << 8) | (m_data[j + 3]);
        }

        for (uint32_t k = 16; k < 64; ++k)
        {
            m[k] = sig1(m[k - 2]) + m[k - 7] + sig0(m[k - 15]) + m[k - 16];
        }

        for (uint32_t i = 0; i < 8; ++i)
        {
            state[i] = m_state[i];
        }

        for (uint32_t i = 0; i < 64; ++i)
        {
            maj = majority(state[0], state[1], state[2]);
            xorA = rotr(state[0], 2) ^ rotr(state[0], 13) ^ rotr(state[0], 22);
            ch = choose(state[4], state[5], state[6]);
            xorE = rotr(state[4], 6) ^ rotr(state[4], 11) ^ rotr(state[4], 25);

            sum = m[i] + k[i] + state[7] + ch + xorE;
            newA = xorA + maj;
            newE = state[3] + sum;

            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = newE;
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = sum + newA;
        }

        for (uint32_t i = 0; i < 8; ++i)
        {
            m_state[i] += state[i];
        }
    }

    void reset()
    {
        m_state[0] = 0x6a09e667;
        m_state[1] = 0xbb67ae85;
        m_state[2] = 0x3c6ef372;
        m_state[3] = 0xa54ff53a;
        m_state[4] = 0x510e527f;
        m_state[5] = 0x9b05688c;
        m_state[6] = 0x1f83d9ab;
        m_state[7] = 0x5be0cd19;
        m_blocklen = 0;
        m_bitlen = 0;
    }
};

constexpr uint32_t SHA256::k[64];

int main()
{
    SHA256 sha;
    std::string data = "hello world";
    sha.update(data);
    std::cout << "SHA-256: " << sha.final() << std::endl;
    return 0;
}
