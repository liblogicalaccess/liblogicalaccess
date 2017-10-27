#include "logicalaccess/crypto/des_helper.hpp"
#include <cassert>
#include <memory>
#include <logicalaccess/crypto/des_cipher.hpp>
#include <logicalaccess/crypto/des_symmetric_key.hpp>
#include <logicalaccess/crypto/des_initialization_vector.hpp>

using namespace logicalaccess;


ByteVector DESHelper::DESEncrypt(const ByteVector &data, const ByteVector &key,
                                 const ByteVector &iv_data)
{
    return DESRun(data, key, iv_data, true);
}

ByteVector DESHelper::DESDecrypt(const ByteVector &data, const ByteVector &key,
                                 ByteVector const &iv_data)
{
    return DESRun(data, key, iv_data, false);
}

ByteVector DESHelper::DESRun(const ByteVector &data, const ByteVector &key,
                             const ByteVector &iv_data, bool crypt)
{
    assert(key.size() != 0 && key.size() % 8 == 0);
    assert(iv_data.size() % 8 == 0);
    assert(data.size() % 8 == 0);

    auto cipher = std::make_shared<openssl::DESCipher>();
    auto isokey = std::make_shared<openssl::DESSymmetricKey>(
        openssl::DESSymmetricKey::createFromData(key));

    std::shared_ptr<openssl::DESInitializationVector> iv;

    if (iv_data.size() == 0)
        iv = std::make_shared<openssl::DESInitializationVector>(
            openssl::DESInitializationVector::createNull());
    else
        iv = std::make_shared<openssl::DESInitializationVector>(
            openssl::DESInitializationVector::createFromData(iv_data));

    ByteVector result;
    if (crypt)
        cipher->cipher(data, result, *isokey.get(), *iv.get(), false);
    else
        cipher->decipher(data, result, *isokey.get(), *iv.get(), false);
    return result;
}
