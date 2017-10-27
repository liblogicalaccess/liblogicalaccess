#include "epasscrypto.hpp"
#include "utils.hpp"
#include <cassert>
#include <cstring>
#include <logicalaccess/crypto/cmac.hpp>
#include <logicalaccess/crypto/des_cipher.hpp>
#include <logicalaccess/crypto/des_helper.hpp>
#include <logicalaccess/crypto/des_initialization_vector.hpp>
#include <logicalaccess/crypto/des_symmetric_key.hpp>
#include <logicalaccess/crypto/lla_random.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>

using namespace logicalaccess;

EPassCrypto::EPassCrypto(const std::string &mrz)
    : step2_success_(false)
{
    auto seed = EPassUtils::seed_from_mrz(mrz);
    k_enc_    = EPassUtils::compute_enc_key(seed);
    k_mac_    = EPassUtils::compute_mac_key(seed);
}

ByteVector EPassCrypto::step1(const ByteVector &random_icc,
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

    auto E_ifd = DESHelper::DESEncrypt(S, k_enc_, {});
    auto M_ifd = EPassUtils::compute_mac(EPassUtils::pad(E_ifd), k_mac_);

    auto ret = E_ifd;
    ret.insert(ret.end(), M_ifd.begin(), M_ifd.end());

    return ret;
}

bool EPassCrypto::step2(const ByteVector &auth_response)
{
    assert(!step2_success_);
    auto clear = DESHelper::DESDecrypt(auth_response, k_enc_, {});
    if (auth_response.size() != 40)
        return false;

    ByteVector random_ifd(clear.begin() + 8, clear.begin() + 16);
    if (random_ifd != random_ifd_)
        return false;

    ByteVector random_icc(clear.begin(), clear.begin() + 8);
    ByteVector k_icc(clear.begin() + 16, clear.begin() + 32);
    ByteVector k_seed;
    for (int i = 0; i < 16; ++i)
        k_seed.push_back(k_icc[i] ^ random_k_ifd_[i]);

    S_enc_ = EPassUtils::compute_enc_key(k_seed);
    S_mac_ = EPassUtils::compute_mac_key(k_seed);

    S_send_counter_.insert(S_send_counter_.end(), random_icc.begin() + 4,
                           random_icc.end());
    S_send_counter_.insert(S_send_counter_.end(), random_ifd_.begin() + 4,
                           random_ifd_.end());

    step2_success_ = true;
    return true;
}

ByteVector EPassCrypto::get_session_enc_key() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_enc_;
}

ByteVector EPassCrypto::get_session_mac_key() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_mac_;
}

ByteVector EPassCrypto::get_send_session_counter() const
{
    EXCEPTION_ASSERT_WITH_LOG(
        step2_success_, LibLogicalAccessException,
        "Authentication Step 2 has not successfully completed. You are now allowed "
        "to call this method.");

    return S_send_counter_;
}

bool EPassCrypto::secureMode() const
{
    return step2_success_;
}

ByteVector EPassCrypto::encrypt_apdu(const ByteVector &apdu)
{
    auto ret        = EPassUtils::encrypt_apdu(apdu, S_enc_, S_mac_, S_send_counter_);
    S_send_counter_ = EPassUtils::increment_ssc(S_send_counter_);

    return ret;
}

ByteVector EPassCrypto::decrypt_rapdu(const ByteVector &rapdu)
{
    auto ret        = EPassUtils::decrypt_rapdu(rapdu, S_enc_, S_mac_, S_send_counter_);
    S_send_counter_ = EPassUtils::increment_ssc(S_send_counter_);

    return ret;
}
