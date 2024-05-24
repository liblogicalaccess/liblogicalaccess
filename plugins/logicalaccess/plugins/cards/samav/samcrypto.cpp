/**
 * \file samcrypto.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMDESfireCrypto source.
 */

#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/crypto/tomcrypt.h>
#include <ctime>
#include <cstdlib>
#include <climits>
#include <logicalaccess/myexception.hpp>

#include <cstring>

#include <openssl/rand.h>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>

namespace logicalaccess
{
SAMDESfireCrypto::SAMDESfireCrypto()
{
}

SAMDESfireCrypto::~SAMDESfireCrypto()
{
}

ByteVector SAMDESfireCrypto::authenticateHostP1(std::shared_ptr<DESFireKey> key,
                                                ByteVector encRndB,
                                                unsigned char /*keyno*/)
{
    ByteVector keyvec = key->getData();

    if (key->getKeyType() == DF_KEY_AES)
        d_cipher.reset(new openssl::AESCipher());
    else if (key->getKeyType() == DF_KEY_3K3DES || key->getKeyType() == DF_KEY_DES)
        d_cipher.reset(new openssl::DESCipher());

    openssl::AESSymmetricKey cipherkey = openssl::AESSymmetricKey::createFromData(keyvec);
    openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
    d_rndB.clear();
    d_cipher->decipher(encRndB, d_rndB, cipherkey, iv, false);

    ByteVector rndB1;
    rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
    rndB1.push_back(d_rndB[0]);

    d_rndA.clear();
    d_rndA.resize(16);
    if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector rndAB;
    rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
    rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

    ByteVector ret;
    d_cipher->cipher(rndAB, ret, cipherkey, iv, false);
    return ret;
}

void SAMDESfireCrypto::authenticateHostP2(unsigned char keyno, ByteVector encRndA1,
                                          std::shared_ptr<DESFireKey> key)
{
    ByteVector keyvec = key->getData();
    ByteVector checkRndA;
    ByteVector rndA;
    openssl::AESSymmetricKey cipherkey = openssl::AESSymmetricKey::createFromData(keyvec);
    openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
    d_cipher->decipher(encRndA1, rndA, cipherkey, iv, false);

    d_sessionKey.clear();
    checkRndA.push_back(rndA[15]);
    checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 15);

    if (d_rndA == checkRndA)
    {
        if (key->getKeyType() == DF_KEY_3K3DES)
        {
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 6,
                                d_rndA.begin() + 10);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 6,
                                d_rndB.begin() + 10);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12,
                                d_rndA.begin() + 16);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12,
                                d_rndB.begin() + 16);

            d_cipher.reset(new openssl::DESCipher());
            d_authkey    = keyvec;
            d_mac_size   = 8;
        }
        else if (key->getKeyType() == DF_KEY_DES)
        {
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4,
                                d_rndA.begin() + 8);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4,
                                d_rndB.begin() + 8);

            d_cipher.reset(new openssl::DESCipher());
            d_authkey    = keyvec;
            d_mac_size   = 8;
        }
        else
        {
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12,
                                d_rndA.begin() + 16);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12,
                                d_rndB.begin() + 16);

            d_cipher.reset(new openssl::AESCipher());
            d_authkey    = keyvec;
            d_mac_size   = 8;
        }

        d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHostP2 Failed!");
}

ByteVector SAMDESfireCrypto::sam_crc_encrypt(ByteVector d_sessionKey,
                                             ByteVector vectordata,
                                             std::shared_ptr<DESFireKey> key) const
{
    ByteVector ret;
    std::shared_ptr<openssl::SymmetricKey> cipherkey;
    std::shared_ptr<openssl::InitializationVector> iv;
    long crc;
    char crc_size;

    if (key->getKeyType() == DF_KEY_AES)
    {
        cipherkey.reset(new openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(d_sessionKey)));
        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createNull()));
        crc      = desfire_crc32(&vectordata[0], vectordata.size());
        crc_size = 4;
    }
    else
    {
        cipherkey.reset(new openssl::DESSymmetricKey(
            openssl::DESSymmetricKey::createFromData(d_sessionKey)));
        iv.reset(new openssl::DESInitializationVector(
            openssl::DESInitializationVector::createNull()));
        crc      = desfire_crc16(&vectordata[0], vectordata.size());
        crc_size = 2;
    }

    for (char x = 0; x < crc_size; ++x)
    {
        vectordata.push_back(static_cast<unsigned char>(crc & 0xff));
        crc >>= 8;
    }
    if (vectordata.size() % d_cipher->getBlockSize() != 0)
        vectordata.resize(vectordata.size() + (d_cipher->getBlockSize() -
                               (vectordata.size() % d_cipher->getBlockSize())),
                          0x00);
    d_cipher->cipher(vectordata, ret, *cipherkey, *iv, false);
    return ret;
}
}