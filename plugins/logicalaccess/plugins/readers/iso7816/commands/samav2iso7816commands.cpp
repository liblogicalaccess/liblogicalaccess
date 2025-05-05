/**
 * \file SAMAV2ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */


#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samkucentry.hpp>
#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <cstring>

namespace logicalaccess
{
SAMAV2ISO7816Commands::SAMAV2ISO7816Commands()
    : SAMISO7816Commands<KeyEntryAV2Information, SETAV2>(CMD_SAMAV2ISO7816)
    , d_cmdCtr(0)
{
    d_lastMacIV.resize(16);
}

SAMAV2ISO7816Commands::SAMAV2ISO7816Commands(std::string ct)
    : SAMISO7816Commands<KeyEntryAV2Information, SETAV2>(ct)
    , d_cmdCtr(0)
{
    d_lastMacIV.resize(16);
}

SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
{
}

void SAMAV2ISO7816Commands::generateSessionKey(ByteVector rnda, ByteVector rndb)
{
    ByteVector SV1a(16), SV2a(16), SV1b(16), SV2b(16), emptyIV(16);

    copy(rnda.begin() + 11, rnda.begin() + 16, SV1a.begin());
    copy(rndb.begin() + 11, rndb.begin() + 16, SV1a.begin() + 5);
    copy(rnda.begin() + 4, rnda.begin() + 9, SV1a.begin() + 10);
    for (unsigned char x = 4; x <= 9; ++x)
    {
        SV1a[x + 6] ^= rndb[x];
    }

    copy(rnda.begin() + 10, rnda.begin() + 15, SV1b.begin());
    copy(rndb.begin() + 10, rndb.begin() + 15, SV1b.begin() + 5);
    copy(rnda.begin() + 5, rnda.begin() + 10, SV1b.begin() + 10);
    for (unsigned char x = 5; x <= 10; ++x)
    {
        SV1b[x + 5] ^= rndb[x];
    }

    copy(rnda.begin() + 7, rnda.begin() + 12, SV2a.begin());
    copy(rndb.begin() + 7, rndb.begin() + 12, SV2a.begin() + 5);
    copy(rnda.begin(), rnda.begin() + 5, SV2a.begin() + 10);
    for (unsigned char x = 0; x <= 5; ++x)
    {
        SV2a[x + 10] ^= rndb[x];
    }

    copy(rnda.begin() + 6, rnda.begin() + 11, SV2b.begin());
    copy(rndb.begin() + 6, rndb.begin() + 11, SV2b.begin() + 5);
    copy(rnda.begin() + 1, rnda.begin() + 6, SV2b.begin() + 10);
    for (unsigned char x = 1; x <= 6; ++x)
    {
        SV2b[x + 9] ^= rndb[x];
    }

    size_t sessionKeySize = d_macSessionKey.size();
    if (sessionKeySize == 32) /* AES 256 */
    {
        SV1a[15] = 0x87;
        SV1b[15] = 0x88;
        SV2a[15] = 0x89;
        SV2b[15] = 0x8a;
    }
    else if (sessionKeySize == 24) /* AES 192 */
    {
        SV1a[15] = 0x83;
        SV1b[15] = 0x84;
        SV2a[15] = 0x85;
        SV2b[15] = 0x86;
    }
    else /* AES 128 */
    {
        SV1a[15] = 0x81; 
        SV1b[15] = 0x00;
        SV2a[15] = 0x82;
        SV2b[15] = 0x00;
    }

    std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
    std::shared_ptr<openssl::InitializationVector> iv(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(emptyIV)));
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
    ByteVector Kea, Keb, Kma, Kmb;

    cipher->cipher(SV1a, Kea, *symkey.get(), *iv.get(), false);
    cipher->cipher(SV1b, Keb, *symkey.get(), *iv.get(), false);

    cipher->cipher(SV2a, Kma, *symkey.get(), *iv.get(), false);
    cipher->cipher(SV2b, Kmb, *symkey.get(), *iv.get(), false);

    d_sessionKey = Kea;
    d_macSessionKey = Kma;
    if (sessionKeySize == 32) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), Keb.begin(), Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.begin(), Kmb.end());
    }
    else if (sessionKeySize == 24) /* AES 192 */
    {
        for (unsigned char x = 0; x <= 7; ++x)
        {
            d_sessionKey[x + 8] ^= Keb[x];
            d_macSessionKey[x + 8] ^= Kmb[x];
        }
        d_sessionKey.insert(d_sessionKey.end(), Keb.end() - 8, Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.end() - 8, Kmb.end());
    }
}

void SAMAV2ISO7816Commands::generateOfflineSessionKey(std::shared_ptr<DESFireKey> key, unsigned short changecnt)
{
    ByteVector SV1a, SV2a, SV1b, SV2b, emptyIV(16);

    if (key->getKeyType() != DF_KEY_AES)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "generateOfflineSessionKey Only AES Key allowed.");
    }
    auto keydata = key->getData();

    SV1a.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    SV1a.push_back(static_cast<unsigned char>(changecnt & 0xff));
    SV2a = SV1b = SV2b = SV1a;

    if (keydata.size() == 32) /* AES 256 */
    {
        SV1a.resize(16, 0x77);
        SV1b.resize(16, 0x78);
        SV2a.resize(16, 0x79);
        SV2b.resize(16, 0x7a);
    }
    else if (keydata.size() == 24) /* AES 192 */
    {
        SV1a.resize(16, 0x73);
        SV1b.resize(16, 0x74);
        SV2a.resize(16, 0x75);
        SV2b.resize(16, 0x76);
    }
    else /* AES 128 */
    {
        SV1a.resize(16, 0x71);
        SV1b.resize(16, 0x00);
        SV2a.resize(16, 0x72);
        SV2b.resize(16, 0x00);
    }

    std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(keydata)));
    std::shared_ptr<openssl::InitializationVector> iv(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(emptyIV)));
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
    ByteVector Kea, Keb, Kma, Kmb;

    cipher->cipher(SV1a, Kea, *symkey.get(), *iv.get(), false);
    cipher->cipher(SV1b, Keb, *symkey.get(), *iv.get(), false);

    cipher->cipher(SV2a, Kma, *symkey.get(), *iv.get(), false);
    cipher->cipher(SV2b, Kmb, *symkey.get(), *iv.get(), false);

    d_sessionKey = Kea;
    d_macSessionKey = Kma;
    if (keydata.size() == 32) /* AES 256 */
    {
        d_sessionKey.insert(d_sessionKey.end(), Keb.begin(), Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.begin(), Kmb.end());
    }
    else if (keydata.size() == 24) /* AES 192 */
    {
        for (unsigned char x = 0; x <= 7; ++x)
        {
            d_sessionKey[x + 8] ^= Keb[x];
            d_macSessionKey[x + 8] ^= Kmb[x];
        }
        d_sessionKey.insert(d_sessionKey.end(), Keb.end() - 8, Keb.end());
        d_macSessionKey.insert(d_macSessionKey.end(), Kmb.end() - 8, Kmb.end());
    }
}

void SAMAV2ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key,
                                             unsigned char keyno)
{
    unsigned char hostmode = 2;
    ByteVector emptyIV(16);
    ByteVector data_p1(3, 0x00);

    if (key->getKeyType() != DF_KEY_AES)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost Only AES Key allowed.");

    /* emptyIV and Clear Key */
    d_lastMacIV     = emptyIV;
    d_LastSessionIV = emptyIV;
    d_sessionKey.clear();
    d_macSessionKey.clear();

    data_p1[0] = keyno;
    data_p1[1] = key->getKeyVersion();
    data_p1[2] = hostmode; // Host Mode: Full Protection

    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        d_cla, 0xa4, 0x00, 0x00, 0x03, data_p1, 0x00);
    if (result.getData().size() != 12 || result.getSW1() != 0x90 || result.getSW2() != 0xAF)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P1 Failed.");

    ByteVector keycipher = key->getData();
    d_macSessionKey = keycipher;
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
    ByteVector rnd1;

    /* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
    ByteVector rnd2 = result.getData();
    rnd2.push_back(hostmode); // Host Mode: Full Protection
    rnd2.resize(16);          // ZeroPad

    ByteVector macHost =
        openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd2, d_lastMacIV, 16);
    truncateMacBuffer(macHost);

    rnd1.resize(12);
    if (RAND_bytes(&rnd1[0], static_cast<int>(rnd1.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector data_p2;
    data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + 8);
    data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());

    result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x14,
                                                            data_p2, 0x00);
    if (result.getData().size() != 24 || result.getSW1() != 0x90 || result.getSW2() != 0xAF)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P2 Failed.");

    /* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
    rnd1.insert(rnd1.end(), rnd2.begin() + 12, rnd2.end()); // p2 data without rnd2

    macHost =
        openssl::CMACCrypto::cmac(d_macSessionKey, cipher, rnd1, d_lastMacIV, 16);
    truncateMacBuffer(macHost);

    for (unsigned char x = 0; x < 8; ++x)
    {
        if (macHost[x] != result.getData()[x])
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "authenticateHost P2 CMAC from SAM is Wrong.");
    }

    /* Create kxe - d_authKey */
    generateAuthEncKey(keycipher, rnd1, rnd2);

    // create rndA
    ByteVector rndA(16);
    if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    // decipher rndB
    std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_authKey)));
    std::shared_ptr<openssl::InitializationVector> iv(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_lastMacIV)));

    ByteVector encRndB(result.getData().begin() + 8, result.getData().end());
    ByteVector dencRndB;

    cipher->decipher(encRndB, dencRndB, *symkey.get(), *iv.get(), false);

    // create rndB'
    ByteVector rndB1;
    rndB1.insert(rndB1.begin(), dencRndB.begin() + 2, dencRndB.begin() + dencRndB.size());
    rndB1.push_back(dencRndB[0]);
    rndB1.push_back(dencRndB[1]);

    ByteVector dataHost, encHost;
    dataHost.insert(dataHost.end(), rndA.begin(), rndA.end());   // RndA
    dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); // RndB'

    iv.reset(new openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(d_lastMacIV)));
    cipher->cipher(dataHost, encHost, *symkey.get(), *iv.get(), false);

    result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x20,
                                                            encHost, 0x00);
    if (result.getData().size() != 16 || result.getSW1() != 0x90 || result.getSW2() != 0x00)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P3 Failed.");

    ByteVector SAMrndA;
    iv.reset(new openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(d_lastMacIV)));
    cipher->decipher(result.getData(), SAMrndA, *symkey.get(), *iv.get(), false);
    SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

    if (!equal(SAMrndA.begin(), SAMrndA.begin() + 16, rndA.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost P3 RndA from SAM is invalid.");

    generateSessionKey(rndA, dencRndB);
    d_cmdCtr = 0;
}

ByteVector SAMAV2ISO7816Commands::createfullProtectionCmd(ByteVector cmd)
{
    bool lc, le;
    unsigned char lcvalue;
    std::shared_ptr<openssl::SymmetricKey> symkeySession(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_sessionKey)));
    std::shared_ptr<openssl::InitializationVector> ivSession(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_LastSessionIV)));
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
    ByteVector protectedCmd = cmd, cmdCtrVector, finalFullProtectedCmd = cmd, encData;

    getLcLe(cmd, lc, lcvalue, le);

    if (!lc)
    {
        protectedCmd.insert(protectedCmd.begin() + AV2_LC_POS, 0x00);
        finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_LC_POS, 0x00);
    }
    else
    {
        ByteVector data(cmd.begin() + AV2_HEADER_LENGTH,
                        cmd.begin() + AV2_HEADER_LENGTH + lcvalue);
        protectedCmd.erase(protectedCmd.begin() + AV2_HEADER_LENGTH,
                           protectedCmd.begin() + AV2_HEADER_LENGTH + lcvalue);
        finalFullProtectedCmd.erase(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH,
                                    finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH +
                                        lcvalue);

        if (data.size() % 16 != 0)
        {
            data.push_back(0x80);
            if (data.size() % 16 != 0)
                data.resize((unsigned char)(data.size() / 16 + 1) * 16);
        }

        /* generate IV because first encrypt */
        d_LastSessionIV = generateEncIV(true);
        ivSession.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_LastSessionIV)));

        cipher->cipher(data, encData, *symkeySession.get(), *ivSession.get(), false);
        protectedCmd.insert(protectedCmd.begin() + AV2_HEADER_LENGTH, encData.begin(),
                            encData.end());
        finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH,
                                     encData.begin(), encData.end());
    }

    protectedCmd[4]          = static_cast<unsigned char>(encData.size()) + 8;
    finalFullProtectedCmd[4] = static_cast<unsigned char>(encData.size()) + 8;

    /* Set counter*/
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    protectedCmd.insert(protectedCmd.begin() + 2, cmdCtrVector.begin(),
                        cmdCtrVector.end());

    if (lc)
    {
        /* Creater our cipher buffer and keep last block */
        std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
        std::shared_ptr<openssl::InitializationVector> ivMac(
            new openssl::AESInitializationVector(
                openssl::AESInitializationVector::createFromData(d_lastMacIV)));
        unsigned char blockReady = (unsigned char)(protectedCmd.size() / 16) * 16;
        ByteVector encMac(protectedCmd.begin(), protectedCmd.begin() + blockReady), tmp;
        protectedCmd.erase(protectedCmd.begin(), protectedCmd.begin() + blockReady);

        cipher->cipher(encMac, tmp, *symkeyMac.get(), *ivMac.get(), false);
        d_lastMacIV.assign(tmp.end() - 16, tmp.end());
    }

    ByteVector encProtectedCmd = openssl::CMACCrypto::cmac(d_macSessionKey, cipher,
                                                           protectedCmd, d_lastMacIV, 16);
    truncateMacBuffer(encProtectedCmd);

    finalFullProtectedCmd.insert(finalFullProtectedCmd.begin() + AV2_HEADER_LENGTH +
                                     encData.size(),
                                 encProtectedCmd.begin(), encProtectedCmd.begin() + 8);
    return finalFullProtectedCmd;
}

void SAMAV2ISO7816Commands::getLcLe(ByteVector cmd, bool &lc, unsigned char &lcvalue,
                                    bool &le)
{
    if (cmd.size() < AV2_HEADER_LENGTH)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "getLcLe cmd param is too little.");

    // CLA INS P1 P2
    if (cmd.size() == AV2_LC_POS)
    {
        lc      = false;
        le      = false;
        lcvalue = 0;
    }
    // CLA INS P1 P2 LE
    else if (cmd.size() == AV2_HEADER_LENGTH)
    {
        lc      = false;
        le      = true;
        lcvalue = 0;
    }
    // CLA INS P1 P2 LC DATA
    else if (cmd[AV2_LC_POS] + AV2_HEADER_LENGTH == static_cast<int>(cmd.size()))
    {
        lc      = true;
        le      = false;
        lcvalue = cmd[AV2_LC_POS];
    }
    // CLA INS P1 P2 LC DATA LE
    else if (cmd[AV2_LC_POS] + AV2_HEADER_LENGTH_WITH_LE == static_cast<int>(cmd.size()))
    {
        lc      = true;
        le      = true;
        lcvalue = cmd[AV2_LC_POS];
    }
}

ByteVector SAMAV2ISO7816Commands::verifyAndDecryptResponse(ByteVector response)
{
    std::shared_ptr<openssl::SymmetricKey> symkeySession(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_sessionKey)));
    std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_macSessionKey)));
    std::shared_ptr<openssl::InitializationVector> ivMac(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_lastMacIV)));
    std::shared_ptr<openssl::InitializationVector> ivSession(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_LastSessionIV)));
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

    /* begin check mac */
    ByteVector myMac, cmdCtrVector, myEncMac, data;
    if (response.size() < 2 + 8)
        return response;

    ByteVector mac(response.end() - 8 - 2, response.end() - 2);

    myMac.push_back(response[response.size() - 2]);
    myMac.push_back(response[response.size() - 1]);

    /* Set counter*/
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    reverse(cmdCtrVector.begin(), cmdCtrVector.end());
    myMac.insert(myMac.end(), cmdCtrVector.begin(), cmdCtrVector.end());

    if (response.size() != 2 + 8)
    {
        /* We have data Creater our cipher buffer and keep last block */
        myMac.insert(myMac.end(), response.begin(), response.end() - 10);
        unsigned char blockReady = (unsigned char)(myMac.size() / 16) * 16;
        ByteVector lastBlock(myMac.begin() + blockReady, myMac.end());
        myMac.erase(myMac.begin() + blockReady, myMac.end());

        cipher->cipher(myMac, myEncMac, *symkeyMac.get(), *ivMac.get(), false);
        d_lastMacIV.assign(myEncMac.end() - 16, myEncMac.end());
        myMac = lastBlock;
    }

    myEncMac =
        openssl::CMACCrypto::cmac(d_macSessionKey, cipher, myMac, d_lastMacIV, 16);
    truncateMacBuffer(myEncMac);

    if (!equal(myEncMac.begin(), myEncMac.begin() + 8, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "verifyAndDecryptResponse wasnt able to verify the answer of the sam");

    if (response.size() > 2 + 8)
    {
        /* begin decrypt */

        /* generate IV because first decrypt */
        d_LastSessionIV = generateEncIV(false);
        ivSession.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_LastSessionIV)));
        ByteVector encData(response.begin(), response.end() - 2 - 8);

        cipher->decipher(encData, data, *symkeySession.get(), *ivSession.get(), false);

        int i = (int)data.size() - 1;
        for (; i >= 0 && data[i] != 0x80 && data[i] == 0x00; --i)
        {
        }
        if (i >= 0)
            data.resize(i);
    }
    data.push_back(response[response.size() - 2]);
    data.push_back(response[response.size() - 1]);
    return data;
}

ByteVector SAMAV2ISO7816Commands::generateEncIV(bool encrypt) const
{
    ByteVector myIV(4), cmdCtrVector, encIV;
    BufferHelper::setUInt32(cmdCtrVector, d_cmdCtr);
    reverse(cmdCtrVector.begin(), cmdCtrVector.end());

    if (encrypt)
    {
        fill(myIV.begin(), myIV.end(), 0x01);
        for (unsigned char i = 1; i < 4; ++i)
            myIV.insert(myIV.end(), cmdCtrVector.begin(), cmdCtrVector.end());
    }
    else
    {
        fill(myIV.begin(), myIV.end(), 0x02);
        for (unsigned char i = 1; i < 4; ++i)
            myIV.insert(myIV.end(), cmdCtrVector.begin(), cmdCtrVector.end());
    }

    std::shared_ptr<openssl::SymmetricKey> symkeyMac(new openssl::AESSymmetricKey(
        openssl::AESSymmetricKey::createFromData(d_sessionKey)));
    std::shared_ptr<openssl::InitializationVector> iv(
        new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(d_LastSessionIV)));
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

    cipher->cipher(myIV, encIV, *symkeyMac.get(), *iv.get(), false);
    return encIV;
}

ByteVector SAMAV2ISO7816Commands::transmit(ByteVector cmd, bool first, bool last)
{
    ByteVector result;

    if (d_sessionKey.size())
    {
        try
        {
            result =
                getISO7816ReaderCardAdapter()->sendCommand(createfullProtectionCmd(cmd));
        }
        catch (std::exception)
        {
            fill(d_sessionKey.begin(), d_sessionKey.end(), 0);
            fill(d_macSessionKey.begin(), d_macSessionKey.end(), 0);
            fill(d_LastSessionIV.begin(), d_LastSessionIV.end(), 0);
            fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
            throw;
        }

        if (first)
        {
            fill(d_LastSessionIV.begin(), d_LastSessionIV.end(), 0x00);
            fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
            ++d_cmdCtr;
        }
        result = verifyAndDecryptResponse(result);
        if (last)
        {
            fill(d_LastSessionIV.begin(), d_LastSessionIV.end(), 0x00);
            fill(d_lastMacIV.begin(), d_lastMacIV.end(), 0);
        }
    }
    else
        result = getISO7816ReaderCardAdapter()->sendCommand(cmd);
    return result;
}

std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>>
SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
{
    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> keyentry;
    KeyEntryAV2Information keyentryinformation;
    unsigned char cmd[] = {d_cla, 0x64, keyno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);

    ByteVector result = transmit(cmd_vector, true, true);

    if ((result.size() == 15 || result.size() == 14) &&
        result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        keyentry.reset(new SAMKeyEntry<KeyEntryAV2Information, SETAV2>());
        keyentryinformation.ExtSET = result[result.size() - 3];
        memcpy(keyentryinformation.set, &result[result.size() - 5], 2);
        keyentry->setSET(keyentryinformation.set);

        keyentryinformation.kuc          = result[result.size() - 6];
        keyentryinformation.cekv         = result[result.size() - 7];
        keyentryinformation.cekno        = result[result.size() - 8];
        keyentryinformation.desfirekeyno = result[result.size() - 9];

        memcpy(keyentryinformation.desfireAid, &result[result.size() - 12], 3);

        if (result.size() == 14)
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
    unsigned char cmd[] = {d_cla, 0x6c, kucno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() == 12 &&
        (result[result.size() - 2] == 0x90 || result[result.size() - 1] == 0x00))
    {
        SAMKUCEntryStruct kucentrys;
        memcpy(&kucentrys, &result[0], sizeof(SAMKUCEntryStruct));
        kucentry->setKucEntryStruct(kucentrys);
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKUCEntry failed.");
    return kucentry;
}

void SAMAV2ISO7816Commands::changeKUCEntry(unsigned char kucno,
                                           std::shared_ptr<SAMKucEntry> kucentry,
                                           std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char cmd[] = {d_cla, 0xcc, kucno, kucentry->getUpdateMask(), 0x06};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(),
                      reinterpret_cast<char *>(&kucentry->getKucEntryStruct()),
                      reinterpret_cast<char *>(&kucentry->getKucEntryStruct()) + 6);
    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
}

void SAMAV2ISO7816Commands::changeKeyEntry(
    unsigned char keyno,
    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> keyentry,
    std::shared_ptr<DESFireKey> /*key*/)
{
    if (d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char proMas = keyentry->getUpdateMask();

    size_t buffer_size  = SAM_KEY_BUFFER_SIZE + sizeof(KeyEntryAV2Information);
    unsigned char *data = new unsigned char[buffer_size]();

    memcpy(data, keyentry->getData(), SAM_KEY_BUFFER_SIZE);
    memcpy(data + SAM_KEY_BUFFER_SIZE, &keyentry->getKeyEntryInformation(),
           sizeof(KeyEntryAV2Information));
    ByteVector vectordata(data, data + buffer_size);
    delete[] data;

    unsigned char cmd[] = {d_cla, 0xc1, keyno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
}

void SAMAV2ISO7816Commands::changeKeyEntryOffline(
	unsigned char keyno,
	const KeyEntryUpdateSettings& updateSettings,
	unsigned short changecnt,
	const ByteVector& encke)
{
	unsigned char proMas = SAMBasicKeyEntry::getUpdateMask(updateSettings);

	ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
	vectordata.insert(vectordata.end(), encke.begin(), encke.end());

    unsigned char cmd[] = {d_cla, 0xc1, keyno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntryOffline failed.")
}

void SAMAV2ISO7816Commands::changeKUCEntryOffline(
	unsigned char kucno,
	const KucEntryUpdateSettings& updateSettings,
	unsigned short changecnt,
	const ByteVector& enckuc)
{
	unsigned char proMas = SAMKucEntry::getUpdateMask(updateSettings);
	
	ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
	vectordata.insert(vectordata.end(), enckuc.begin(), enckuc.end());
	
	unsigned char cmd[] = {d_cla, 0xcc, kucno, proMas, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntryOffline failed.")
}

void SAMAV2ISO7816Commands::disableKeyEntryOffline(
	unsigned char keyno,
	unsigned short changecnt,
	const ByteVector& encuid)
{
	ByteVector vectordata;
    vectordata.push_back(static_cast<unsigned char>((changecnt >> 8) & 0xff));
    vectordata.push_back(static_cast<unsigned char>(changecnt & 0xff));
	vectordata.insert(vectordata.end(), encuid.begin(), encuid.end());
	
	unsigned char cmd[] = {d_cla, 0xd8, keyno, 0x00, (unsigned char)vectordata.size()};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), vectordata.begin(), vectordata.end());

    ByteVector result = transmit(cmd_vector, true, true);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "disableKeyEntryOffline failed.")
}

ByteVector SAMAV2ISO7816Commands::dumpSecretKey(unsigned char keyno,
                                                unsigned char keyversion,
                                                ByteVector divInpu)
{
    unsigned char p1 = 0x00;

    if (divInpu.size())
        p1 |= 0x02;

    unsigned char cmd[] = {
        d_cla, 0xd6,       p1,  0x00, static_cast<unsigned char>(divInpu.size() + 0x02),
        keyno, keyversion, 0x00};
    ByteVector cmd_vector(cmd, cmd + 8);
    cmd_vector.insert(cmd_vector.end() - 1, divInpu.begin(), divInpu.end());

    ByteVector result = transmit(cmd_vector);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "dumpSecretKey failed.");

    return ByteVector(result.begin(), result.end() - 2);
}

void SAMAV2ISO7816Commands::activateOfflineKey(unsigned char keyno,
                                               unsigned char keyversion,
                                               ByteVector divInpu)
{
    ByteVector activateOfflineKey = {0x80,
                                     0x01,
                                     static_cast<unsigned char>(divInpu.size() > 0x00),
                                     0x00,
                                     static_cast<unsigned char>(0x02 + divInpu.size()),
                                     keyno,
                                     keyversion};
    activateOfflineKey.insert(activateOfflineKey.end(), divInpu.begin(), divInpu.end());

    transmit(activateOfflineKey);
}

ByteVector SAMAV2ISO7816Commands::decipherOfflineData(ByteVector data)
{
    ByteVector decipherOfflineData = {
        0x80, 0x0d, 0x00, 0x00, static_cast<unsigned char>(data.size()), 0x00,
    };
    decipherOfflineData.insert(decipherOfflineData.end() - 1, data.begin(), data.end());

    auto result = transmit(decipherOfflineData);
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 2, LibLogicalAccessException,
                              "Response is too short");
    result.resize(result.size() - 2);
    return result;
}

ByteVector SAMAV2ISO7816Commands::encipherOfflineData(ByteVector data)
{
    ByteVector encipherOfflineData = {
        0x80, 0x0e, 0x00, 0x00, static_cast<unsigned char>(data.size()), 0x00,
    };
    encipherOfflineData.insert(encipherOfflineData.end() - 1, data.begin(), data.end());

    auto result = transmit(encipherOfflineData);
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 2, LibLogicalAccessException,
                              "Response is too short");
    result.resize(result.size() - 2);
    return result;
}

ByteVector SAMAV2ISO7816Commands::cmacOffline(const ByteVector &data)
{
    unsigned int block_size = 16;
    unsigned char Rb        = 0x87;

    ByteVector blankbuf;
    blankbuf.resize(block_size, 0x00);
    ByteVector L = encipherOfflineData(blankbuf);

    ByteVector K1;
    if ((L[0] & 0x80) == 0x00)
    {
        K1 = openssl::CMACCrypto::shift_string(L);
    }
    else
    {
        K1 = openssl::CMACCrypto::shift_string(L, Rb);
    }

    ByteVector K2;
    if ((K1[0] & 0x80) == 0x00)
    {
        K2 = openssl::CMACCrypto::shift_string(K1);
    }
    else
    {
        K2 = openssl::CMACCrypto::shift_string(K1, Rb);
    }

    int pad = (block_size - (data.size() % block_size)) % block_size;
    if (data.size() == 0)
        pad = block_size;

    ByteVector padded_data = data;
    if (pad > 0)
    {
        padded_data.push_back(0x80);
        if (pad > 1)
        {
            for (int i = 0; i < (pad - 1); ++i)
            {
                padded_data.push_back(0x00);
            }
        }
    }

    // XOR with K1
    if (pad == 0)
    {
        for (unsigned int i = 0; i < K1.size(); ++i)
        {
            padded_data[padded_data.size() - K1.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K1.size() + i] ^ K1[i]);
        }
    }
    // XOR with K2
    else
    {
        for (unsigned int i = 0; i < K2.size(); ++i)
        {
            padded_data[padded_data.size() - K2.size() + i] = static_cast<unsigned char>(
                padded_data[padded_data.size() - K2.size() + i] ^ K2[i]);
        }
    }

    ByteVector cmac = encipherOfflineData(padded_data);
    if (cmac.size() > block_size)
    {
        cmac = ByteVector(cmac.end() - block_size, cmac.end());
    }

    return cmac;
}
}