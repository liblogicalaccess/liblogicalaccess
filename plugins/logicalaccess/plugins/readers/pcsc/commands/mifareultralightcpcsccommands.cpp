/**
 * \file mifareultralightcpcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C PC/SC commands.
 */

#include <logicalaccess/plugins/readers/pcsc/commands/mifareultralightcpcsccommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcchip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>
#include <logicalaccess/cards/readermemorykeystorage.hpp>
#include <logicalaccess/cards/samkeystorage.hpp>

#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
MifareUltralightCPCSCCommands::MifareUltralightCPCSCCommands()
    : MifareUltralightPCSCCommands(CMD_MIFAREULTRALIGHTCPCSC)
{
}

MifareUltralightCPCSCCommands::MifareUltralightCPCSCCommands(std::string ct)
    : MifareUltralightPCSCCommands(ct)
{
}

MifareUltralightCPCSCCommands::~MifareUltralightCPCSCCommands()
{
}

void MifareUltralightCPCSCCommands::startGenericSession()
{
}

void MifareUltralightCPCSCCommands::stopGenericSession()
{
}

std::shared_ptr<MifareUltralightChip>
MifareUltralightCPCSCCommands::getMifareUltralightChip()
{
    return MifareUltralightCommands::getMifareUltralightChip();
}

void MifareUltralightCPCSCCommands::writePage(int page, const ByteVector &buf)
{
    MifareUltralightPCSCCommands::writePage(page, buf);
}

ByteVector MifareUltralightCPCSCCommands::sendGenericCommand(const ByteVector & /*data*/)
{
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented function call.");
}

ByteVector MifareUltralightCPCSCCommands::authenticate_PICC1()
{
    ByteVector data;
    data.push_back(0x1A);
    data.push_back(0x00);

    return sendGenericCommand(data);
}

ByteVector MifareUltralightCPCSCCommands::authenticate_PICC2(const ByteVector &encRndAB)
{
    ByteVector data;
    data.push_back(0xAF);
    data.insert(data.end(), encRndAB.begin(), encRndAB.end());

    return sendGenericCommand(data);
}

void MifareUltralightCPCSCCommands::authenticate(std::shared_ptr<TripleDESKey> authkey)
{
    ByteVector result;

    if (!authkey || authkey->isEmpty())
    {
        authkey = getMifareUltralightCChip()->getDefaultKey();
    }

    startGenericSession();

    try
    {
        // Get RndB from the PICC
        result = authenticate_PICC1();
        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 11, CardException,
                                  "Authentication failed. The PICC return a bad buffer.");
        EXCEPTION_ASSERT_WITH_LOG(result.at(0) == 0xAF, CardException,
                                  "Authentication failed. Return code unexcepted.");

        ByteVector iv;
        iv.resize(8, 0x00);

        ByteVector key =
            ByteVector(authkey->getData(), authkey->getData() + authkey->getLength());
        ByteVector encRndB(result.begin() + 1, result.end() - 2);
        ByteVector rndB = DESFireCrypto::desfire_CBC_receive(key, iv, encRndB);

        ByteVector rndA;
        rndA.resize(8);
        if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Cannot retrieve cryptographically strong bytes");
        }

        ByteVector rndAB;
        rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
        rndAB.insert(rndAB.end(), rndB.begin() + 1, rndB.end());
        rndAB.push_back(rndB.at(0));

        ByteVector encRndAB = DESFireCrypto::sam_CBC_send(key, encRndB, rndAB);

        // Send Ek(RndAB) to the PICC and get RndA'
        result = authenticate_PICC2(encRndAB);
        EXCEPTION_ASSERT_WITH_LOG(result.size() >= 11, CardException,
                                  "Authentication failed. The PICC return a bad buffer.");
        EXCEPTION_ASSERT_WITH_LOG(result.at(0) == 0x00, CardException,
                                  "Authentication failed. Return code unexcepted.");

        iv = ByteVector(encRndAB.end() - 8, encRndAB.end());
        ByteVector encRndA1(result.begin() + 1, result.end() - 2);
        ByteVector rndA1 = DESFireCrypto::desfire_CBC_receive(key, iv, encRndA1);
        rndA1.insert(rndA1.begin(), rndA1.at(rndA1.size() - 1));
        rndA1.erase(rndA1.end() - 1);

        EXCEPTION_ASSERT_WITH_LOG(rndA == rndA1, CardException,
                                  "Authentication failed. RndA' != RndA.");
    }
    catch (std::exception &)
    {
        stopGenericSession();
        throw;
    }

    stopGenericSession();
}
}