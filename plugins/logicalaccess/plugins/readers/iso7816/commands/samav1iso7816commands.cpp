/**
 * \file SAMAV1ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav1iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samav1chip.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samkucentry.hpp>
#include <logicalaccess/plugins/cards/samav/samcommands.hpp>
#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/thread/thread_time.hpp>
#include <cstring>

namespace logicalaccess
{
SAMAV1ISO7816Commands::SAMAV1ISO7816Commands()
    : SAMISO7816Commands<KeyEntryAV1Information, SETAV1>(CMD_SAMAV1ISO7816)
{
}

SAMAV1ISO7816Commands::SAMAV1ISO7816Commands(std::string ct)
    : SAMISO7816Commands<KeyEntryAV1Information, SETAV1>(ct)
{
}

SAMAV1ISO7816Commands::~SAMAV1ISO7816Commands()
{
}

std::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1>>
SAMAV1ISO7816Commands::getKeyEntry(unsigned char keyno)
{
    unsigned char cmd[] = {d_cla, 0x64, keyno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);
    std::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1>> keyentry;
    KeyEntryAV1Information keyentryinformation;
    memset(&keyentryinformation, 0x00, sizeof(KeyEntryAV1Information));

    ByteVector result = transmit(cmd_vector);

    if ((result.size() == 14 || result.size() == 13) &&
        result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        keyentry.reset(new SAMKeyEntry<KeyEntryAV1Information, SETAV1>());

        memcpy(keyentryinformation.set, &result[result.size() - 4], 2);
        keyentry->setSET(keyentryinformation.set);

        keyentryinformation.kuc          = result[result.size() - 5];
        keyentryinformation.cekv         = result[result.size() - 6];
        keyentryinformation.cekno        = result[result.size() - 7];
        keyentryinformation.desfirekeyno = result[result.size() - 8];

        memcpy(keyentryinformation.desfireAid, &result[result.size() - 11], 3);

        if (result.size() == 13)
        {
            keyentryinformation.verb = result[result.size() - 12];
            keyentryinformation.vera = result[result.size() - 13];
        }
        else
        {
            keyentryinformation.verc = result[result.size() - 12];
            keyentryinformation.verb = result[result.size() - 13];
            keyentryinformation.vera = result[result.size() - 14];
        }

        keyentry->setKeyEntryInformation(keyentryinformation);
        keyentry->setKeyTypeFromSET();
        keyentry->setUpdateMask(0);
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
    return keyentry;
}

std::shared_ptr<SAMKucEntry> SAMAV1ISO7816Commands::getKUCEntry(unsigned char kucno)
{
    std::shared_ptr<SAMKucEntry> kucentry(new SAMKucEntry);
    unsigned char cmd[] = {d_cla, 0x6c, kucno, 0x00, 0x00};
    ByteVector cmd_vector(cmd, cmd + 5);

    ByteVector result = transmit(cmd_vector);

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

void SAMAV1ISO7816Commands::changeKeyEntry(
    unsigned char keyno,
    std::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1>> keyentry,
    std::shared_ptr<DESFireKey> key)
{
    if (d_crypto->d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char proMas = keyentry->getUpdateMask();

    size_t buffer_size  = SAM_KEY_BUFFER_SIZE + sizeof(KeyEntryAV1Information);
    unsigned char *data = new unsigned char[buffer_size]();

    memcpy(data, keyentry->getData(), SAM_KEY_BUFFER_SIZE);
    memcpy(data + SAM_KEY_BUFFER_SIZE, &keyentry->getKeyEntryInformation(),
           sizeof(KeyEntryAV1Information));

    ByteVector iv;
    iv.resize(16, 0x00);

    ByteVector vectordata(data, data + buffer_size);
    delete[] data;

    ByteVector encdatalittle;

    if (key->getKeyType() == DF_KEY_DES)
        encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
    else
        encdatalittle =
            d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

    unsigned char cmd[] = {d_cla, 0xc1, keyno, proMas,
                           (unsigned char)(encdatalittle.size())};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), encdatalittle.begin(), encdatalittle.end());

    ByteVector result = transmit(cmd_vector);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
}

void SAMAV1ISO7816Commands::authenticateHost(std::shared_ptr<DESFireKey> key,
                                             unsigned char keyno)
{
    if (key->getKeyType() == DF_KEY_DES)
        authenticateHostDES(key, keyno);
    else
        authenticateHost_AES_3K3DES(key, keyno);
}

void SAMAV1ISO7816Commands::authenticateHost_AES_3K3DES(std::shared_ptr<DESFireKey> key,
                                                        unsigned char keyno)
{
    ByteVector data;
    unsigned char authMode = 0x00;

    data.push_back(keyno);
    data.push_back(key->getKeyVersion());

    unsigned char cmdp1[] = {d_cla, 0xa4, authMode, 0x00, 0x02, 0x00};
    ByteVector cmd_vector(cmdp1, cmdp1 + 6);
    cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

    ByteVector result = transmit(cmd_vector);
    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0xaf))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost_AES_3K3DES P1 failed.");

    ByteVector encRndB(result.begin(), result.end() - 2);
    ByteVector encRndAB = d_crypto->authenticateHostP1(key, encRndB, keyno);

    unsigned char cmdp2[] = {d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()),
                             0x00};
    cmd_vector.assign(cmdp2, cmdp2 + 6);
    cmd_vector.insert(cmd_vector.end() - 1, encRndAB.begin(), encRndAB.end());

    result = transmit(cmd_vector);
    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHost_AES_3K3DES P2 failed.");

    ByteVector encRndA1(result.begin(), result.end() - 2);
    d_crypto->authenticateHostP2(keyno, encRndA1, key);
}

void SAMAV1ISO7816Commands::authenticateHostDES(std::shared_ptr<DESFireKey> key,
                                                unsigned char keyno)
{
    ByteVector data;
    unsigned char authMode = 0x00;

    data.push_back(keyno);
    data.push_back(key->getKeyVersion());

    ByteVector cmd_vector = {d_cla, 0xa4, authMode, 0x00, 0x02, 0x00};
    cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

    ByteVector result = transmit(cmd_vector);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0xaf))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHostDES P1 failed.");

    ByteVector keyvec = key->getData();

    // get encRNB
    ByteVector encRNB(result.begin(), result.end() - 2);

    // dec RNB
    ByteVector RndB;
    openssl::DESSymmetricKey cipherkey = openssl::DESSymmetricKey::createFromData(keyvec);
    openssl::DESInitializationVector iv = openssl::DESInitializationVector::createNull();
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::DESCipher());

    cipher->decipher(encRNB, RndB, cipherkey, iv, false);

    // Create RNB'
    ByteVector rndB1;
    rndB1.insert(rndB1.end(), RndB.begin() + 1, RndB.begin() + RndB.size());
    rndB1.push_back(RndB[0]);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");
    // Create our RndA
    ByteVector rndA(8);
    if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    // create rndAB
    ByteVector rndAB;
    rndAB.clear();
    rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
    rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

    // enc rndAB
    ByteVector encRndAB;
    cipher->cipher(rndAB, encRndAB, cipherkey, iv, false);

    // send enc rndAB
    cmd_vector = {d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), 0x00};
    cmd_vector.insert(cmd_vector.end() - 1, encRndAB.begin(), encRndAB.end());

    result = transmit(cmd_vector);
    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHostDES P2 failed.");

    ByteVector encRndA1(result.begin(), result.end() - 2);
    ByteVector dencRndA1;
    cipher->decipher(encRndA1, dencRndA1, cipherkey, iv, false);

    // create rndA'
    ByteVector rndA1;
    rndA1.insert(rndA1.end(), rndA.begin() + 1, rndA.begin() + rndA.size());
    rndA1.push_back(rndA[0]);

    // Check if RNDA is our
    if (!equal(dencRndA1.begin(), dencRndA1.end(), rndA1.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "authenticateHostDES Final Check failed.");

    d_crypto->d_sessionKey.clear();

    d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(),
                                  rndA.begin() + 4);
    d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(),
                                  RndB.begin() + 4);
    d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(),
                                  rndA.begin() + 4);
    d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(),
                                  RndB.begin() + 4);
}

void SAMAV1ISO7816Commands::changeKUCEntry(unsigned char kucno,
                                           std::shared_ptr<SAMKucEntry> kucentry,
                                           std::shared_ptr<DESFireKey> key)
{
    if (d_crypto->d_sessionKey.size() == 0)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Failed: AuthentificationHost have to be done before use such command.");

    unsigned char data[6] = {};
    memcpy(data, &kucentry->getKucEntryStruct(), 6);
    ByteVector vectordata(data, data + 6);
    ByteVector encdatalittle;

    if (key->getKeyType() == DF_KEY_DES)
        encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
    else
        encdatalittle =
            d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

    unsigned char proMas = kucentry->getUpdateMask();

    unsigned char cmd[] = {d_cla, 0xcc, kucno, proMas, 0x08};
    ByteVector cmd_vector(cmd, cmd + 5);
    cmd_vector.insert(cmd_vector.end(), encdatalittle.begin(), encdatalittle.end());
    ByteVector result = transmit(cmd_vector);

    if (result.size() >= 2 &&
        (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
}
}
