/**
 * \file SAMAV2ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav2iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav2/samcrypto.hpp"
#include "samav2/samkeyentry.hpp"
#include "samav2/samkucentry.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/cmac.hpp"

#include <cstring>

namespace logicalaccess
{
    SAMAV2ISO7816Commands::SAMAV2ISO7816Commands() : SAMAV2Commands<KeyEntryAV2Information, SETAV2>(CMD_SAMAV2ISO7816), d_cmdCtr(0)
    {
        d_lastMacIV.resize(16);
		bridgeISO = std::make_shared<SAMISO7816Commands<KeyEntryAV2Information, SETAV2>>();

		auto tmp = new SAMISO7816Commands<KeyEntryAV2Information, SETAV2>();
    }

	SAMAV2ISO7816Commands::SAMAV2ISO7816Commands(std::string ct) : SAMAV2Commands<KeyEntryAV2Information, SETAV2>(ct), d_cmdCtr(0)
	{
		d_lastMacIV.resize(16);
		bridgeISO = std::make_shared<SAMISO7816Commands<KeyEntryAV2Information, SETAV2>>();
	}


    SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
    {
    }

    void SAMAV2ISO7816Commands::generateSessionKey(std::vector<unsigned char> rnda, std::vector<unsigned char> rndb)
    {
        std::vector<unsigned char> SV1a(16), SV2a(16), SV1b(16), emptyIV(16);

        std::copy(rnda.begin() + 11, rnda.begin() + 16, SV1a.begin());
        std::copy(rndb.begin() + 11, rndb.begin() + 16, SV1a.begin() + 5);
        std::copy(rnda.begin() + 4, rnda.begin() + 9, SV1a.begin() + 10);

        for (unsigned char x = 4; x <= 9; ++x)
        {
            SV1a[x + 6] ^= rndb[x];
        }

        std::copy(rnda.begin() + 7, rnda.begin() + 12, SV2a.begin());
        std::copy(rndb.begin() + 7, rndb.begin() + 12, SV2a.begin() + 5);
        std::copy(rnda.begin(), rnda.begin() + 5, SV2a.begin() + 10);

        for (unsigned char x = 0; x <= 5; ++x)
        {
            SV2a[x + 10] ^= rndb[x];
        }

        SV1a[15] = 0x81; /* AES 128 */
        SV2a[15] = 0x82; /* AES 128 */
        /* TODO AES 192 */

        std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
        std::shared_ptr<openssl::InitializationVector> iv(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(emptyIV)));
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

        cipher->cipher(SV1a, getSamIsoCmd()->getSessionKeyRef(), *symkey.get(), *iv.get(), false);
        cipher->cipher(SV2a, d_macSessionKey, *symkey.get(), *iv.get(), false);
    }

    void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno)
    {
        unsigned char hostmode = 2;
        std::vector<unsigned char> result, emptyIV(16);
        std::vector<unsigned char> data_p1(3, 0x00);

        if (key->getKeyType() != DESFireKeyType::DF_KEY_AES)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost Only AES Key allowed.");

        /* emptyIV and Clear Key */
        d_lastMacIV = emptyIV;
        getSamIsoCmd()->getLastSessionIVRef() = emptyIV;
        getSamIsoCmd()->getSessionKeyRef().clear();
        d_macSessionKey.clear();

        data_p1[0] = keyno;
        data_p1[1] = key->getKeyVersion();
        data_p1[2] = hostmode; //Host Mode: Full Protection

        result = getSamIsoCmd()->getISO7816ReaderCardAdapter()->sendAPDUCommand(getSamIsoCmd()->getClaRef(), 0xa4, 0x00, 0x00, 0x03, data_p1, 0x00);
        if (result.size() != 14 || result[12] != 0x90 || result[13] != 0xAF)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost P1 Failed.");

        std::vector<unsigned char> keycipher(key->getData(), key->getData() + key->getLength());
        d_macSessionKey = keycipher;
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
        std::vector<unsigned char> rnd1;

        /* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
        std::vector<unsigned char>  rnd2(result.begin(), result.begin() + 12);
        rnd2.push_back(hostmode); //Host Mode: Full Protection
        rnd2.resize(16); //ZeroPad

        std::vector<unsigned char> macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, 16, rnd2, d_lastMacIV, 16);
		getSamIsoCmd()->truncateMacBuffer(macHost);

        rnd1.resize(12);
        if (RAND_bytes(&rnd1[0], static_cast<int>(rnd1.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        std::vector<unsigned char> data_p2;
        data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + 8);
        data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());

        result = getSamIsoCmd()->getISO7816ReaderCardAdapter()->sendAPDUCommand(getSamIsoCmd()->getClaRef(), 0xa4, 0x00, 0x00, 0x14, data_p2, 0x00);
        if (result.size() != 26 || result[24] != 0x90 || result[25] != 0xAF)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost P2 Failed.");

        /* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
        rnd1.insert(rnd1.end(), rnd2.begin() + 12, rnd2.end()); //p2 data without rnd2

        macHost = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, 16, rnd1, d_lastMacIV, 16);
        getSamIsoCmd()->truncateMacBuffer(macHost);

        for (unsigned char x = 0; x < 8; ++x)
        {
            if (macHost[x] != result[x])
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost P2 CMAC from SAM is Wrong.");
        }

        /* Create kxe - getAuthKeyRef() */
		getSamIsoCmd()->generateAuthEncKey(keycipher, rnd1, rnd2);

        //create rndA
        std::vector<unsigned char> rndA(16);
        if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        //decipher rndB
        std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(getSamIsoCmd()->getAuthKeyRef())));
        std::shared_ptr<openssl::InitializationVector> iv(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastMacIV)));

        std::vector<unsigned char> encRndB(result.begin() + 8, result.end() - 2);
        std::vector<unsigned char> dencRndB;

        cipher->decipher(encRndB, dencRndB, *symkey.get(), *iv.get(), false);

        //create rndB'
        std::vector<unsigned char> rndB1;
        rndB1.insert(rndB1.begin(), dencRndB.begin() + 2, dencRndB.begin() + dencRndB.size());
        rndB1.push_back(dencRndB[0]);
        rndB1.push_back(dencRndB[1]);

        std::vector<unsigned char> dataHost, encHost;
        dataHost.insert(dataHost.end(), rndA.begin(), rndA.end()); //RndA
        dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); //RndB'

        iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastMacIV)));
        cipher->cipher(dataHost, encHost, *symkey.get(), *iv.get(), false);

        result = getSamIsoCmd()->getISO7816ReaderCardAdapter()->sendAPDUCommand(getSamIsoCmd()->getClaRef(), 0xa4, 0x00, 0x00, 0x20, encHost, 0x00);
        if (result.size() != 18 || result[16] != 0x90 || result[17] != 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost P3 Failed.");

        std::vector<unsigned char> encSAMrndA(result.begin(), result.end() - 2), SAMrndA;
        iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastMacIV)));
        cipher->decipher(encSAMrndA, SAMrndA, *symkey.get(), *iv.get(), false);
        SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

        if (!std::equal(SAMrndA.begin(), SAMrndA.begin() + 16, rndA.begin()))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHost P3 RndA from SAM is invalid.");

        generateSessionKey(rndA, dencRndB);
        d_cmdCtr = 0;
    }

    std::vector<unsigned char> SAMAV2ISO7816Commands::createfullProtectionCmd(std::vector<unsigned char> cmd)
    {
        bool lc, le;
        unsigned char lcvalue;
        std::shared_ptr<openssl::SymmetricKey> symkeySession(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(getSamIsoCmd()->getSessionKeyRef())));
        std::shared_ptr<openssl::InitializationVector> ivSession(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(getSamIsoCmd()->getLastSessionIVRef())));
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
        std::vector<unsigned char> protectedCmd = cmd, encProtectedCmd, cmdCtrVector, finalFullProtectedCmd = cmd, encData;

        getLcLe(cmd, lc, lcvalue, le);

        if (!lc)
        {
            protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, 0x00);
            finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_LC_POS, 0x00);
        }
        else
        {
            std::vector<unsigned char> data(cmd.begin() + AV2_HEADER_LENGTH, cmd.begin() + AV2_HEADER_LENGTH + lcvalue);
            protectedCmd.erase(protectedCmd.begin() + AV2_HEADER_LENGTH, protectedCmd.begin() + AV2_HEADER_LENGTH + lcvalue);
            finalFullProtectedCmd.erase(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH, finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH + lcvalue);

            if (data.size() % 16 != 0)
            {
                data.push_back(0x80);
                if (data.size() % 16 != 0)
                    data.resize((unsigned char)(data.size() / 16 + 1) * 16);
            }

            /* generate IV because first encrypt */
            getSamIsoCmd()->getLastSessionIVRef() = generateEncIV(true);
            ivSession.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(getSamIsoCmd()->getLastSessionIVRef())));

            cipher->cipher(data, encData, *symkeySession.get(), *ivSession.get(), false);
            protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(), encData.end());
            finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(), encData.end());
        }

        protectedCmd[4] = (unsigned char)encData.size() + 8;
        finalFullProtectedCmd[4] = (unsigned char)encData.size() + 8;

        /* Set counter*/
        BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
        std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
        protectedCmd.insert(protectedCmd.begin() + 2, cmdCtrVector.begin(), cmdCtrVector.end());

        if (lc)
        {
            /* Creater our cipher buffer and keep last block */
            std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
            std::shared_ptr<openssl::InitializationVector> ivMac(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastMacIV)));
            unsigned char blockReady = (unsigned char)(protectedCmd.size() / 16) * 16;
            std::vector<unsigned char> encMac(protectedCmd.begin(), protectedCmd.begin() + blockReady), tmp;
            protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);

            cipher->cipher(encMac, tmp, *symkeyMac.get(), *ivMac.get(), false);
            d_lastMacIV.assign(tmp.end() - 16, tmp.end());
        }

        encProtectedCmd = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, 16, protectedCmd, d_lastMacIV, 16);
        getSamIsoCmd()->truncateMacBuffer(encProtectedCmd);

        finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH + encData.size(), encProtectedCmd.begin(), encProtectedCmd.begin() + 8);
        return finalFullProtectedCmd;
    }

    void SAMAV2ISO7816Commands::getLcLe(std::vector<unsigned char> cmd, bool& lc, unsigned char& lcvalue, bool& le)
    {
        if (cmd.size() < AV2_HEADER_LENGTH)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getLcLe cmd param is too little.");

        //CLA INS P1 P2
        if (cmd.size() == AV2_LC_POS)
        {
            lc = false;
            le = false;
            lcvalue = 0;
        }
        //CLA INS P1 P2 LE
        else if (cmd.size() == AV2_HEADER_LENGTH)
        {
            lc = false;
            le = true;
            lcvalue = 0;
        }
        //CLA INS P1 P2 LC DATA
        else if (cmd[AV2_LC_POS] + AV2_HEADER_LENGTH == static_cast<int>(cmd.size()))
        {
            lc = true;
            le = false;
            lcvalue = cmd[AV2_LC_POS];
        }
        // CLA INS P1 P2 LC DATA LE
        else if (cmd[AV2_LC_POS] + AV2_HEADER_LENGTH_WITH_LE == static_cast<int>(cmd.size()))
        {
            lc = true;
            le = true;
            lcvalue = cmd[AV2_LC_POS];
        }
    }

    std::vector<unsigned char> SAMAV2ISO7816Commands::verifyAndDecryptResponse(std::vector<unsigned char> response)
    {
        std::shared_ptr<openssl::SymmetricKey> symkeySession(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(getSamIsoCmd()->getSessionKeyRef())));
        std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
        std::shared_ptr<openssl::InitializationVector> ivMac(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastMacIV)));
        std::shared_ptr<openssl::InitializationVector> ivSession(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(getSamIsoCmd()->getLastSessionIVRef())));
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

        /* begin check mac */
        std::vector<unsigned char> myMac, cmdCtrVector, myEncMac, data;
        if (response.size() < 2 + 8)
            return response;

        std::vector<unsigned char> mac(response.end() - 8 - 2, response.end() - 2);

        myMac.push_back(response[response.size() - 2]);
        myMac.push_back(response[response.size() - 1]);

        /* Set counter*/
        BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
        std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());
        myMac.insert(myMac.end(), cmdCtrVector.begin(), cmdCtrVector.end());

        if (response.size() != 2 + 8)
        {
            /* We have data Creater our cipher buffer and keep last block */
            myMac.insert(myMac.end(), response.begin(), response.end() - 10);
            unsigned char blockReady = (unsigned char)(myMac.size() / 16) * 16;
            std::vector<unsigned char> lastBlock(myMac.begin() + blockReady, myMac.end());
            myMac.erase(myMac.begin() + blockReady, myMac.end());

            cipher->cipher(myMac, myEncMac, *symkeyMac.get(), *ivMac.get(), false);
            d_lastMacIV.assign(myEncMac.end() - 16, myEncMac.end());
            myMac = lastBlock;
        }

        myEncMac = openssl::CMACCrypto::cmac(d_macSessionKey, cipher, 16, myMac, d_lastMacIV, 16);
        getSamIsoCmd()->truncateMacBuffer(myEncMac);

        if (!std::equal(myEncMac.begin(), myEncMac.begin() + 8, mac.begin()))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "verifyAndDecryptResponse wasnt able to verify the answer of the sam");

        if (response.size() > 2 + 8)
        {
            /* begin decrypt */

            /* generate IV because first decrypt */
            getSamIsoCmd()->getLastSessionIVRef() = generateEncIV(false);
            ivSession.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(getSamIsoCmd()->getLastSessionIVRef())));
            std::vector<unsigned char> encData(response.begin(), response.end() - 2 - 8);

            cipher->decipher(encData, data, *symkeySession.get(), *ivSession.get(), false);

            int i = (int)data.size() - 1;
            for (; i >= 0 && data[i] != 0x80 && data[i] == 0x00; --i);
            if (i >= 0)
                data.resize(i);
        }
        data.push_back(response[response.size() - 2]);
        data.push_back(response[response.size() - 1]);
        return data;
    }

    std::vector<unsigned char> SAMAV2ISO7816Commands::generateEncIV(bool encrypt)
    {
        std::vector<unsigned char> myIV(4), cmdCtrVector, encIV;
        BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
        std::reverse(cmdCtrVector.begin(), cmdCtrVector.end());

        if (encrypt)
        {
            std::fill(myIV.begin(), myIV.end(), 0x01);
            for (unsigned char i = 1; i < 4; ++i)
                myIV.insert(myIV.end(), cmdCtrVector.begin(), cmdCtrVector.end());
        }
        else
        {
            std::fill(myIV.begin(), myIV.end(), 0x02);
            for (unsigned char i = 1; i < 4; ++i)
                myIV.insert(myIV.end(), cmdCtrVector.begin(), cmdCtrVector.end());
        }

        std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(getSamIsoCmd()->getSessionKeyRef())));
        std::shared_ptr<openssl::InitializationVector> iv(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(getSamIsoCmd()->getLastSessionIVRef())));
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

        cipher->cipher(myIV, encIV, *symkeyMac.get(), *iv.get(), false);
        return encIV;
    }

    std::vector<unsigned char> SAMAV2ISO7816Commands::transmit(std::vector<unsigned char> cmd, bool first, bool last)
    {
        std::vector<unsigned char> result;

        if (getSamIsoCmd()->getSessionKeyRef().size())
        {
            try
            {
                result = getSamIsoCmd()->getISO7816ReaderCardAdapter()->sendCommand(createfullProtectionCmd(cmd));
            }
            catch (std::exception)
            {
                std::fill(getSamIsoCmd()->getSessionKeyRef().begin(), getSamIsoCmd()->getSessionKeyRef().end(), 0);
                std::fill(d_macSessionKey.begin(), d_macSessionKey.end(), 0);
                std::fill(getSamIsoCmd()->getLastSessionIVRef().begin(), getSamIsoCmd()->getLastSessionIVRef().end(), 0);
                std::fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
                throw;
            }

            if (first)
            {
                std::fill(getSamIsoCmd()->getLastSessionIVRef().begin(), getSamIsoCmd()->getLastSessionIVRef().end(), 0x00);
                std::fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
                ++d_cmdCtr;
            }
            result = verifyAndDecryptResponse(result);
            if (last)
            {
                std::fill(getSamIsoCmd()->getLastSessionIVRef().begin(), getSamIsoCmd()->getLastSessionIVRef().end(), 0x00);
                std::fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
            }
        }
        else
            result = getSamIsoCmd()->getISO7816ReaderCardAdapter()->sendCommand(cmd);
        return result;
    }

    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
    {
        std::vector<unsigned char> result;
        std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry;
        KeyEntryAV2Information keyentryinformation;
        unsigned char cmd[] = { getSamIsoCmd()->getClaRef(), 0x64, keyno, 0x00, 0x00 };
        std::vector<unsigned char> cmd_vector(cmd, cmd + 5);

        result = transmit(cmd_vector, true, true);

        if ((result.size() == 15 || result.size() == 14) && result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
        {
            keyentry.reset(new SAMKeyEntry<KeyEntryAV2Information, SETAV2>());
            keyentryinformation.ExtSET = result[result.size() - 3];
            memcpy(keyentryinformation.set, &result[result.size() - 5], 2);
            keyentry->setSET(keyentryinformation.set);

            keyentryinformation.kuc = result[result.size() - 6];
            keyentryinformation.cekv = result[result.size() - 7];
            keyentryinformation.cekno = result[result.size() - 8];
            keyentryinformation.desfirekeyno = result[result.size() - 9];

            memcpy(keyentryinformation.desfireAid, &result[result.size() - 12], 3);

            if (result.size() == 13)
            {
                keyentryinformation.verb = result[result.size() - 13];
                keyentryinformation.vera = result[result.size() - 14];
            }
            else
            {
                keyentryinformation.verc = result[result.size() - 13];
                keyentryinformation.verb = result[result.size() - 14];
                keyentryinformation.vera = result[result.size() - 15];
            }

            keyentry->setKeyEntryInformation(keyentryinformation);
            keyentry->setKeyTypeFromSET();
            keyentry->setUpdateMask(0);
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
        return keyentry;
    }

    std::shared_ptr<SAMKucEntry> SAMAV2ISO7816Commands::getKUCEntry(unsigned char kucno)
    {
        std::shared_ptr<SAMKucEntry> kucentry(new SAMKucEntry);
        unsigned char cmd[] = { getSamIsoCmd()->getClaRef(), 0x6c, kucno, 0x00, 0x00 };
        std::vector<unsigned char> cmd_vector(cmd, cmd + 5), result;

        result = transmit(cmd_vector, true, true);

        if (result.size() == 12 && (result[result.size() - 2] == 0x90 || result[result.size() - 1] == 0x00))
        {
            SAMKUCEntryStruct kucentrys;
            memcpy(&kucentrys, &result[0], sizeof(SAMKUCEntryStruct));
            kucentry->setKucEntryStruct(kucentrys);
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKUCEntry failed.");
        return kucentry;
    }

    void SAMAV2ISO7816Commands::changeKUCEntry(unsigned char kucno, std::shared_ptr<SAMKucEntry> kucentry, std::shared_ptr<DESFireKey> key)
    {
        if (getSamIsoCmd()->getSessionKeyRef().size() == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

        unsigned char cmd[] = { getSamIsoCmd()->getClaRef(), 0xcc, kucno, kucentry->getUpdateMask(), 0x06 };
        std::vector<unsigned char> cmd_vector(cmd, cmd + 5), result;
        cmd_vector.insert(cmd_vector.end(), reinterpret_cast<char*>(&kucentry->getKucEntryStruct()), reinterpret_cast<char*>(&kucentry->getKucEntryStruct()) + 6);
        result = transmit(cmd_vector, true, true);

        if (result.size() >= 2 && (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
    }

    void SAMAV2ISO7816Commands::changeKeyEntry(unsigned char keyno, std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, std::shared_ptr<DESFireKey> key)
    {
        if (getSamIsoCmd()->getSessionKeyRef().size() == 0)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

        std::vector<unsigned char> result;
        unsigned char proMas = keyentry->getUpdateMask();

        size_t buffer_size = keyentry->getLength() + sizeof(KeyEntryAV2Information);
        unsigned char *data = new unsigned char[buffer_size]();

        memcpy(data, &*(keyentry->getData()), keyentry->getLength());
        memcpy(data + 48, &keyentry->getKeyEntryInformation(), sizeof(KeyEntryAV2Information));
        std::vector<unsigned char> vectordata(data, data + buffer_size);
        delete[] data;

        unsigned char cmd[] = { getSamIsoCmd()->getClaRef(), 0xc1, keyno, proMas, (unsigned char)vectordata.size() };
        std::vector<unsigned char> cmd_vector(cmd, cmd + 5);
        cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

        result = transmit(cmd_vector, true, true);

        if (result.size() >= 2 && (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
    }

    std::vector<unsigned char> SAMAV2ISO7816Commands::dumpSecretKey(unsigned char keyno, unsigned char keyversion, std::vector<unsigned char> divInpu)
    {
        unsigned char p1 = 0x00;

        if (divInpu.size())
            p1 |= 0x02;

        unsigned char cmd[] = { getSamIsoCmd()->getClaRef(), 0xd6, p1, 0x00, static_cast<unsigned char>(divInpu.size() + 0x02), keyno, keyversion, 0x00 };
        std::vector<unsigned char> cmd_vector(cmd, cmd + 8), result;
        cmd_vector.insert(cmd_vector.end() - 1, divInpu.begin(), divInpu.end());

        result = transmit(cmd_vector);

        if (result.size() >= 2 && (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "dumpSecretKey failed.");

        return std::vector<unsigned char>(result.begin(), result.end() - 2);
    }
}