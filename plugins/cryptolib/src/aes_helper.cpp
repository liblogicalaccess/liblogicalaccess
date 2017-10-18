//
// Created by xaqq on 7/2/15.
//

#include "logicalaccess/crypto/aes_helper.hpp"
#include <cassert>
#include <memory>
#include <logicalaccess/crypto/aes_cipher.hpp>
#include <logicalaccess/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/crypto/aes_initialization_vector.hpp>

using namespace logicalaccess;

ByteVector AESHelper::AESEncrypt(const ByteVector &data,
                                 const ByteVector &key,
                                 const ByteVector &iv_data)
{
    return AESRun(data, key, iv_data, true);
}

ByteVector AESHelper::AESDecrypt(const ByteVector &data,
                                 const ByteVector &key,
                                 ByteVector const &iv_data)
{
    return AESRun(data, key, iv_data, false);
}

ByteVector AESHelper::AESRun(const ByteVector &data, const ByteVector &key,
                             const ByteVector &iv_data,
                             bool crypt)
{
    assert(key.size() != 0 && key.size() % 16 == 0);
    assert(iv_data.size() % 16 == 0);
    assert(data.size() % 16 == 0);

    auto cipher = std::make_shared<openssl::AESCipher>();
    auto isokey = std::make_shared<openssl::AESSymmetricKey>(
            openssl::AESSymmetricKey::createFromData(key));

    std::shared_ptr<openssl::AESInitializationVector> iv;

    if (iv_data.size() == 0)
        iv = std::make_shared<openssl::AESInitializationVector>(
                openssl::AESInitializationVector::createNull());
    else
        iv = std::make_shared<openssl::AESInitializationVector>(
                openssl::AESInitializationVector::createFromData(iv_data));

    ByteVector result;
    if (crypt)
        cipher->cipher(data, result, *isokey.get(), *iv.get(), false);
    else
        cipher->decipher(data, result, *isokey.get(), *iv.get(), false);
    return result;
}