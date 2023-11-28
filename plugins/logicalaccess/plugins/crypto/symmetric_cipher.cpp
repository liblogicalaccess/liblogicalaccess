/**
 * \file symmetric_cipher.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime@leosac.com>
 * \brief Symmetric cipher class.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/openssl.hpp>
#include <logicalaccess/plugins/crypto/symmetric_cipher.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>

namespace logicalaccess
{
namespace openssl
{
void SymmetricCipher::cipher(const ByteVector &src, ByteVector &dest,
	const SymmetricKey &key)
{
    cipher(src, dest, key, openssl::InitializationVector(getBlockSize(), false), false);
}

void SymmetricCipher::cipher(const ByteVector &src, ByteVector &dest,
                             const ByteVector &key, const ByteVector &iv)
{
    cipher(src, dest, openssl::SymmetricKey(key),
           iv.size() > 0 ? openssl::InitializationVector(iv)
                         : openssl::InitializationVector(getBlockSize(), false),
           false);
}

void SymmetricCipher::decipher(const ByteVector &src, ByteVector &dest,
                             const SymmetricKey &key)
{
    decipher(src, dest, key, openssl::InitializationVector(getBlockSize(), false), false);
}

void SymmetricCipher::decipher(const ByteVector &src, ByteVector &dest,
                               const ByteVector &key, const ByteVector &iv)
{
    decipher(src, dest, openssl::SymmetricKey(key),
             iv.size() > 0 ? openssl::InitializationVector(iv)
                           : openssl::InitializationVector(getBlockSize(), false),
             false);
}
}
}