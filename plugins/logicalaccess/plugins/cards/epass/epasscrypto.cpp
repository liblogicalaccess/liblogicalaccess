#include <logicalaccess/plugins/cards/epass/epasscrypto.hpp>
#include <logicalaccess/plugins/cards/epass/utils.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

using namespace logicalaccess;

EPassCrypto::EPassCrypto()
    : ISO24727Crypto()
{
}

EPassCrypto::EPassCrypto(const std::string &mrz)
{
    reset(mrz);
}

void EPassCrypto::reset(const std::string &mrz)
{
    ISO24727Crypto::reset();

    auto seed = EPassUtils::seed_from_mrz(mrz);
    k_enc_    = compute_enc_key(seed);
    k_mac_    = compute_mac_key(seed);
}

ByteVector EPassCrypto::adjust_key_parity(const ByteVector &key)
{
    ByteVector ret;

    for (const auto &c : key)
    {
        bool even = true;
        for (int i = 0; i < 8; ++i)
        {
            if ((c >> i) & 0x01)
                even = !even;
        }
        if (even)
            ret.push_back(static_cast<uint8_t>(c ^ 0x01));
        else
            ret.push_back(c);
    }
    return ret;
}

ByteVector EPassCrypto::compute_enc_key(const ByteVector &seed,
                                        const std::string hash_algo, unsigned char keylen)
{
    ByteVector c = {0, 0, 0, 1};
    ByteVector D = seed;
    D.insert(D.end(), c.begin(), c.end());

    auto H = hash_data(D, hash_algo);
    return adjust_key_parity(ByteVector(H.begin(), H.begin() + keylen));
}

ByteVector EPassCrypto::compute_mac_key(const ByteVector &seed,
                                        const std::string hash_algo, unsigned char keylen)
{
    ByteVector c = {0, 0, 0, 2};
    ByteVector D = seed;
    D.insert(D.end(), c.begin(), c.end());

    auto H = hash_data(D, hash_algo);
    return adjust_key_parity(ByteVector(H.begin(), H.begin() + keylen));
}

void EPassCrypto::compute_session_keys(const ByteVector &k_icc,
                                       const ByteVector &random_icc)
{
    ByteVector k_seed;
    for (int i = 0; i < 16; ++i)
        k_seed.push_back(k_icc[i] ^ random_k_ifd_[i]);

    S_enc_ = compute_enc_key(k_seed, hash_);
    S_mac_ = compute_mac_key(k_seed, hash_);

    S_send_counter_.clear();
    S_send_counter_.insert(S_send_counter_.end(), random_icc.begin() + 4,
                           random_icc.end());
    S_send_counter_.insert(S_send_counter_.end(), random_ifd_.begin() + 4,
                           random_ifd_.end());
}

ByteVector EPassCrypto::compute_mac(std::shared_ptr<openssl::SymmetricCipher> cipher,
                                    const ByteVector &in, const ByteVector &k_mac,
                                    const ByteVector &iv, const ByteVector &ssc)
{
    EXCEPTION_ASSERT_WITH_LOG(in.size() % cipher->getBlockSize() == 0,
                              LibLogicalAccessException,
                              "Data for checksum computation is of improper length.");
    ByteVector kmac_a(k_mac.begin(), k_mac.begin() + 8);
    ByteVector kmac_b(k_mac.begin() + 8, k_mac.end());
    ByteVector y;

    if (ssc.size())
    {
        cipher->cipher(ssc, y, kmac_a, iv);
    }
    else
        y = ByteVector(cipher->getBlockSize(), 0);

    auto itr = in.begin();
    for (uint32_t i = 0; i < in.size() / cipher->getBlockSize(); ++i)
    {
        ByteVector block(itr, itr + cipher->getBlockSize());
        itr += 8;
        // xor
        for (int j = 0; j < 8; ++j)
            y[j] ^= block[j];

        cipher->cipher(y, y, kmac_a, iv);
    }
    cipher->decipher(y, y, kmac_b, iv);
    cipher->cipher(y, y, kmac_a, iv);

    return y;
}