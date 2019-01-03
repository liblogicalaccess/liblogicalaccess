#include <logicalaccess/plugins/crypto/iso24727crypto.hpp>
#include <cassert>
#include <cstring>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/lla_random.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

using namespace logicalaccess;

ISO24727Crypto::ISO24727Crypto(const std::string &cipher,
                               const std::string &hash)
{
    cipher_ = cipher;
    hash_   = hash;
    reset();
}

std::shared_ptr<openssl::SymmetricCipher> ISO24727Crypto::createCipher() const
{
    if (cipher_ == "aes")
        return std::make_shared<openssl::AESCipher>();

	return std::make_shared<openssl::DESCipher>();
}

ByteVector ISO24727Crypto::step1(const ByteVector &random_icc,
                              ByteVector random_ifd /* = {} */,
                              ByteVector random_k_ifd /* = {} */)
{
    EXCEPTION_ASSERT_WITH_LOG(random_icc.size() == 8, LibLogicalAccessException,
                              "Random ICC must be 8 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(random_ifd.size() == 0 || random_ifd.size() == 8,
                              LibLogicalAccessException,
                              "Random IFD must be 8 bytes or empty.");
    EXCEPTION_ASSERT_WITH_LOG(random_k_ifd.size() == 0 || random_k_ifd.size() == 16,
                              LibLogicalAccessException,
                              "Random KIFD must be 16 bytes or empty.");

    random_ifd_ = random_ifd;
    if (random_ifd_.size() == 0)
        random_ifd_ = RandomHelper::bytes(8);

    random_k_ifd_ = random_k_ifd;
    if (random_k_ifd_.size() == 0)
        random_k_ifd_ = RandomHelper::bytes(16);

    ByteVector S = random_ifd_;
    S.insert(S.end(), random_icc.begin(), random_icc.end());
    S.insert(S.end(), random_k_ifd_.begin(), random_k_ifd_.end());

	auto cipher = createCipher();
    ByteVector E_ifd;
    cipher->cipher(S, E_ifd, openssl::SymmetricKey(k_enc_));
    auto M_ifd = compute_mac(cipher, auth_pad(E_ifd), k_mac_);

    auto ret = E_ifd;
    ret.insert(ret.end(), M_ifd.begin(), M_ifd.end());

    return ret;
}

bool ISO24727Crypto::step2(const ByteVector &auth_response)
{
    assert(!step2_success_);

    auto cipher = createCipher();
    if (auth_response.size() < 40)
	{
        LOG(logicalaccess::LogLevel::ERRORS)
            << "Wrong ISO24727 auth response length.";
        return false;
	}
    auto e_icc  = ByteVector(auth_response.begin(),
                            auth_response.begin() + 32);
    ByteVector R; // RND.ICC || RND.IFD || K.ICC
    cipher->decipher(e_icc, R, openssl::SymmetricKey(k_enc_));
    ByteVector m_icc(auth_response.begin() + 32, auth_response.begin() + 40);

	auto m_icc2 = compute_mac(cipher, auth_pad(e_icc), k_mac_);
	if (m_icc2 != m_icc)
	{
        LOG(logicalaccess::LogLevel::ERRORS)
            << "ISO24727 step2 auth response failed: CMAC doesn't match.";
        return false;
	}

    ByteVector random_ifd(R.begin() + 8, R.begin() + 16);
    if (random_ifd != random_ifd_)
	{
        LOG(logicalaccess::LogLevel::ERRORS)
            << "ISO24727 step2 auth response failed: random IFD doesn't match.";
        return false;
	}

    ByteVector random_icc(R.begin(), R.begin() + 8);
    ByteVector k_icc(R.begin() + 16, R.begin() + 32);
    
	compute_session_keys(k_icc, random_icc);

    step2_success_ = true;
    return true;
}

ByteVector ISO24727Crypto::get_session_enc_key() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_enc_;
}

ByteVector ISO24727Crypto::get_session_mac_key() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_mac_;
}

ByteVector ISO24727Crypto::get_send_session_counter() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_send_counter_;
}

bool ISO24727Crypto::secureMode() const
{
    return step2_success_;
}

ByteVector ISO24727Crypto::hash_data(const ByteVector &data, const std::string hash_algo)
{
    if (hash_algo == "sha256")
        return openssl::SHA256Hash(data);

    return openssl::SHA1Hash(data);
}

/**
 * Check whether the APDU contains an LE byte.
 */
static bool apdu_has_le(const ByteVector &apdu)
{
    assert(apdu.size() >= 4);
    if (apdu.size() == 4)
        return false;
    auto body_size = distance(apdu.begin() + 4, apdu.end());
    return body_size != apdu[4] + 1;
}

/**
 * Check whether the APDU has any data.
 */
static bool apdu_has_data(const ByteVector &apdu)
{
    assert(apdu.size() >= 4);
    if (apdu.size() == 4)
        return false;
    auto body_size = distance(apdu.begin() + 4, apdu.end());
    return body_size >= apdu[4] + 1; // Maybe we also have an LE byte.
}

ByteVector ISO24727Crypto::encrypt_apdu(std::shared_ptr<openssl::SymmetricCipher> cipher,
                                       const ByteVector &apdu, const ByteVector &ks_enc,
                                       const ByteVector &ks_mac, const ByteVector &ssc)
{
    EXCEPTION_ASSERT_WITH_LOG(apdu.size() >= 4, LibLogicalAccessException,
                              "APDU is too short to be valid.");

    auto ssc_incremented        = increment_ssc(ssc);
    ByteVector cmd_header_nopad = {0x0C};
    cmd_header_nopad.insert(cmd_header_nopad.end(), apdu.begin() + 1, apdu.begin() + 4);
    ByteVector cmd_header = pad(cmd_header_nopad);

    ByteVector do_97;
    ByteVector original_data;
    if (apdu_has_le(apdu))
    {
        if (apdu_has_data(apdu))
        {
            original_data = ByteVector(apdu.begin() + 4, apdu.end() - 1);
        }
        do_97 = {0x97, 0x01, *(apdu.end() - 1)};
    }
    else if (apdu_has_data(apdu))
    {
        original_data = ByteVector(apdu.begin() + 5, apdu.end());
    }
    ByteVector encrypted_data;
    cipher->cipher(pad(original_data), encrypted_data, openssl::SymmetricKey(ks_enc));

    ByteVector do_87;
    if (apdu_has_data(apdu)) // LC -- do we have any data?
    {
        do_87 = {0x87, static_cast<uint8_t>(encrypted_data.size() + 1), 0x01};
        do_87.insert(do_87.end(), encrypted_data.begin(), encrypted_data.end());
    }

    ByteVector M;
    M.insert(M.end(), cmd_header.begin(), cmd_header.end());
    M.insert(M.end(), do_87.begin(), do_87.end());
    M.insert(M.end(), do_97.begin(), do_97.end());
    M.insert(M.begin(), ssc_incremented.begin(), ssc_incremented.end());

    ByteVector CC    = compute_mac(cipher, pad(M), ks_mac);
    ByteVector do_8E = {0x8E, 0x08};
    do_8E.insert(do_8E.end(), CC.begin(), CC.end());

    ByteVector result;
    result.insert(result.end(), cmd_header_nopad.begin(), cmd_header_nopad.end());
    result.push_back(
        static_cast<uint8_t>(do_87.size() + do_97.size() + do_8E.size())); // LC
    result.insert(result.end(), do_87.begin(), do_87.end());
    result.insert(result.end(), do_97.begin(), do_97.end());
    result.insert(result.end(), do_8E.begin(), do_8E.end());
    result.push_back(0);

    return result;
}

ByteVector ISO24727Crypto::auth_pad(const ByteVector &data)
{
    return ISO24727Crypto::pad(data);
}

ByteVector ISO24727Crypto::pad(const ByteVector &in)
{
    auto out = in;
    out.push_back(1 << 7);

    while (out.size() % 8 != 0)
        out.push_back(0);
    return out;
}

ByteVector ISO24727Crypto::unpad(const ByteVector &in)
{
    auto out = in;

    while (true)
    {
        bool found = (out.back() == 0x80);
        out.pop_back();
        if (found)
            break;
    }
    return out;
}

ByteVector ISO24727Crypto::increment_ssc(const ByteVector &in)
{
    assert(in.size() == 8);
    ByteVector ret;
    ret.reserve(8);

    uint64_t counter;
    for (int i = 0; i < 8; ++i)
        *(reinterpret_cast<uint8_t *>(&counter) + 7 - i) = in[i];
    counter++;

    for (int i = 0; i < 8; ++i)
        ret.push_back((counter >> 8 * (7 - i)) & 0xFF);
    return ret;
}

static bool rapdu_has_data(const ByteVector &rapdu)
{
    EXCEPTION_ASSERT_WITH_LOG(rapdu.size() >= 3, LibLogicalAccessException,
                              "RAPDU is too short.");
    return rapdu[0] == 0x87;
}

ByteVector ISO24727Crypto::decrypt_rapdu(std::shared_ptr<openssl::SymmetricCipher> cipher,
                                        const ByteVector &rapdu, const ByteVector &ks_enc,
                                        const ByteVector &ks_mac, const ByteVector &ssc)
{
    ByteVector do_87;
    ByteVector do_99;
    ByteVector do_8E;

    auto cpy = ByteVector(rapdu.begin(), rapdu.end() - 2);
    auto itr = cpy.begin();
    if (rapdu_has_data(cpy))
    {
        do_87.insert(do_87.end(), itr, itr + 3);
        itr += 3;
        do_87.insert(do_87.end(), itr, itr + cpy[1] - 1);
        itr += cpy[1] - 1;
    }
    EXCEPTION_ASSERT_WITH_LOG(std::distance(itr, cpy.end()) >= 4,
                              LibLogicalAccessException, "RAPDU is too short");
    do_99.insert(do_99.end(), itr, itr + 4);
    itr += 4;
    EXCEPTION_ASSERT_WITH_LOG(std::distance(itr, cpy.end()) >= 10,
                              LibLogicalAccessException, "RAPDU is too short");
    do_8E.insert(do_8E.end(), itr, itr + 10);
    itr += 10;

    ByteVector K;
    auto incremented_ssc = increment_ssc(ssc);
    K.insert(K.end(), incremented_ssc.begin(), incremented_ssc.end());
    K.insert(K.end(), do_87.begin(), do_87.end());
    K.insert(K.end(), do_99.begin(), do_99.end());

    bool rep = false;
    ByteVector CC;
    if (rep)
        CC = compute_mac(cipher, pad(K), ks_mac, {}, ssc);
    else
        CC = compute_mac(cipher, pad(K), ks_mac);
    EXCEPTION_ASSERT_WITH_LOG(CC == ByteVector(do_8E.begin() + 2, do_8E.end()),
                              LibLogicalAccessException, "Checksum doesn't match");

    ByteVector decrypted_data;
    if (do_87.size())
    {
        cipher->decipher(ByteVector(do_87.begin() + 3, do_87.end()), decrypted_data,
                         openssl::SymmetricKey(ks_enc));
        decrypted_data = unpad(decrypted_data);
    }
    decrypted_data.insert(decrypted_data.end(), do_99.begin() + 2, do_99.end());
    return decrypted_data;
}

ByteVector ISO24727Crypto::encrypt_apdu(const ByteVector &apdu)
{
    auto ret        = encrypt_apdu(createCipher(), apdu, S_enc_, S_mac_,
                                            S_send_counter_);
    S_send_counter_ = ISO24727Crypto::increment_ssc(S_send_counter_);

    return ret;
}

ByteVector ISO24727Crypto::decrypt_rapdu(const ByteVector &rapdu)
{
    auto ret        = decrypt_rapdu(createCipher(), rapdu, S_enc_, S_mac_,
                                            S_send_counter_);
    S_send_counter_ = ISO24727Crypto::increment_ssc(S_send_counter_);

    return ret;
}

void ISO24727Crypto::reset()
{
    step2_success_ = false;
    k_enc_.clear();
    k_enc_.clear();
}
