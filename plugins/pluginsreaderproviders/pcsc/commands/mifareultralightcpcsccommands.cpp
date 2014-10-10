/**
 * \file mifareultralightcpcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C PC/SC commands.
 */

#include "../commands/mifareultralightcpcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../pcscreaderprovider.hpp"
#include "mifareultralightcchip.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

#include <openssl/rand.h>
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"

namespace logicalaccess
{
    MifareUltralightCPCSCCommands::MifareUltralightCPCSCCommands()
        : MifareUltralightPCSCCommands()
    {
    }

    MifareUltralightCPCSCCommands::~MifareUltralightCPCSCCommands()
    {
    }

    std::vector<unsigned char> MifareUltralightCPCSCCommands::sendGenericCommand(const std::vector<unsigned char>& data)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented function call.");
    }

    std::vector<unsigned char> MifareUltralightCPCSCCommands::authenticate_PICC1()
    {
        std::vector<unsigned char> data;
        data.push_back(0x1A);
        data.push_back(0x00);

        return sendGenericCommand(data);
    }

    std::vector<unsigned char> MifareUltralightCPCSCCommands::authenticate_PICC2(const std::vector<unsigned char>& encRndAB)
    {
        std::vector<unsigned char> data;
        data.push_back(0xAF);
        data.insert(data.end(), encRndAB.begin(), encRndAB.end());

        return sendGenericCommand(data);
    }

    void MifareUltralightCPCSCCommands::authenticate(boost::shared_ptr<TripleDESKey> authkey)
    {
        std::vector<unsigned char> result;

        if (!authkey || authkey->isEmpty())
        {
            boost::shared_ptr<MifareUltralightCProfile> profile = boost::dynamic_pointer_cast<MifareUltralightCProfile>(getChip()->getProfile());
            if (profile)
            {
                authkey = profile->getKey();
            }
        }

        // Get RndB from the PICC
        result = authenticate_PICC1();
        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 11, CardException, "Authentication failed. The PICC return a bad buffer.");

        openssl::DESCipher cipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);
        unsigned char* keydata = authkey->getData();
        openssl::DESSymmetricKey deskey(openssl::DESSymmetricKey::createFromData(std::vector<unsigned char>(keydata, keydata + authkey->getLength())));
        openssl::DESInitializationVector desiv = openssl::DESInitializationVector::createNull();

        std::vector<unsigned char> encRndB(result.begin() + 1, result.end() - 2);
        std::vector<unsigned char> rndB;
        cipher.decipher(encRndB, rndB, deskey, desiv, false);

        std::vector<unsigned char> rndA;
        rndA.resize(8);
        if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        std::vector<unsigned char> rndAB;
        rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
        rndAB.insert(rndAB.end(), rndB.begin() + 1, rndB.end());
        rndAB.push_back(rndB.at(0));

        std::vector<unsigned char> encRndAB;
        desiv = openssl::DESInitializationVector::createFromData(encRndB);
        cipher.cipher(rndAB, encRndAB, deskey, desiv, false);

        // Send Ek(RndAB) to the PICC and get RndA'
        result = authenticate_PICC2(encRndAB);
        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 10, CardException, "Authentication failed. The PICC return a bad buffer.");

        desiv = openssl::DESInitializationVector::createFromData(std::vector<unsigned char>(encRndAB.end() - 8, encRndAB.end()));
        std::vector<unsigned char> encRndA1(result.begin(), result.end() - 2);
        std::vector<unsigned char> rndA1;
        cipher.decipher(encRndA1, rndA1, deskey, desiv, false);

        EXCEPTION_ASSERT_WITH_LOG(rndA == rndA1, CardException, "Authentication failed. RndA' != RndA.");
    }
}