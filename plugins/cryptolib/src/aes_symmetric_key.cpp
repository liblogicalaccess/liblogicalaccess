/**
 * \file aes_symmetric_key.cpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AES symmetric key class.
 */

#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/sha.hpp"

namespace logicalaccess
{
namespace openssl
{
AESSymmetricKey AESSymmetricKey::createRandom(size_t size)
{
    return AESSymmetricKey(size);
}

AESSymmetricKey AESSymmetricKey::createFromData(const ByteVector &data)
{
    return AESSymmetricKey(data);
}

AESSymmetricKey AESSymmetricKey::createFromPassphrase(const std::string &passphrase)
{
    return createFromData(SHA256Hash(passphrase));
}

AESSymmetricKey::AESSymmetricKey(size_t size)
    : SymmetricKey(size)
{
}

AESSymmetricKey::AESSymmetricKey(const ByteVector &_data)
    : SymmetricKey(_data)
{
}
}
}