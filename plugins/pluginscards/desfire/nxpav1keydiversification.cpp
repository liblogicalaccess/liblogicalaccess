#include "desfirekey.hpp"
#include "logicalaccess/logs.hpp"
#include "nxpav1keydiversification.hpp"
#include <memory>
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/cmac.hpp"
#include "desfirecrypto.hpp"
#include <vector>
#include <boost/property_tree/ptree.hpp>

#include "logicalaccess/crypto/tomcrypt.h"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
void NXPAV1KeyDiversification::initDiversification(ByteVector identifier, int /*AID*/,
                                                   std::shared_ptr<Key> key,
                                                   unsigned char keyno,
                                                   ByteVector &diversify)
{
    diversify.push_back(keyno);
    diversify.insert(diversify.end(), identifier.begin(), identifier.end());

    if (diversify.size() != 8)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "NXP Diversification AV1 need 8 bytes of DivInput (Keyno + 7-byte UID)");
    if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_AES)
        diversify.insert(diversify.end(), diversify.begin(), diversify.end());
}

ByteVector NXPAV1KeyDiversification::getDiversifiedKey(std::shared_ptr<Key> key,
                                                       ByteVector diversify)
{
    LOG(LogLevel::INFOS) << "Using key diversification NXP AV1 with div : "
                         << BufferHelper::getHex(diversify);
    std::shared_ptr<openssl::SymmetricKey> symkey;
    std::shared_ptr<openssl::InitializationVector> iv;
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher;
    ByteVector keycipher(key->getData(), key->getData() + key->getLength());
    ByteVector divKey, divInputEncP1, divInputEncP2;

    if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() != DF_KEY_AES)
    {
        LOG(LogLevel::INFOS) << "Diversification NXP AV1 3DES";
        for (int x       = 0; x < 8; ++x)
            diversify[x] = diversify[x] ^ keycipher[x];

        ByteVector emptyIV(8);
        symkey.reset(new openssl::DESSymmetricKey(
            openssl::DESSymmetricKey::createFromData(keycipher)));
        iv.reset(new openssl::DESInitializationVector(
            openssl::DESInitializationVector::createFromData(emptyIV)));
        cipher.reset(new openssl::DESCipher());

        cipher->cipher(diversify, divInputEncP1, *symkey.get(), *iv.get(), false);

        divKey.insert(divKey.end(), divInputEncP1.begin(), divInputEncP1.end());

        diversify = divInputEncP1;
        for (int x       = 0; x < 8; ++x)
            diversify[x] = diversify[x] ^ keycipher[x + 8];

        cipher->cipher(diversify, divInputEncP2, *symkey.get(), *iv.get(), false);
        divKey.insert(divKey.end(), divInputEncP2.begin(), divInputEncP2.end());
    }
    else if (std::dynamic_pointer_cast<DESFireKey>(key)->getKeyType() == DF_KEY_AES)
    {
        LOG(LogLevel::INFOS) << "Diversification NXP AV1 AES";

        for (int x       = 0; x < 16; ++x)
            diversify[x] = diversify[x] ^ keycipher[x];

        ByteVector emptyIV(16);
        symkey.reset(new openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(keycipher)));
        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(emptyIV)));
        cipher.reset(new openssl::AESCipher());

        cipher->cipher(diversify, divKey, *symkey.get(), *iv.get(), false);
    }
    return divKey;
}

void NXPAV1KeyDiversification::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void NXPAV1KeyDiversification::unSerialize(boost::property_tree::ptree & /*node*/)
{
}
}