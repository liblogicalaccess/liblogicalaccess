/**
 * \file SAMAV1ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMISO7816CARDPROVIDER_HPP

#include <logicalaccess/plugins/cards/samav2/samcommands.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav2/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav2/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav2/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav2/samcommands.hpp>

#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/thread/thread_time.hpp>

#include <string>
#include <vector>
#include <iostream>

#include <logicalaccess/myexception.hpp>

#define DEFAULT_SAM_CLA 0x80

namespace logicalaccess
{
#define CMD_SAMISO7816 "SAMISO7816"

#ifdef SWIG
%template(SAMKeyEntrySETAV1Commands) SAMCommands<KeyEntryAV1Information, SETAV1>;
%template(SAMKeyEntrySETAV2Commands) SAMCommands<KeyEntryAV2Information, SETAV2>;
#endif

/**
 * \brief The SAMISO7816 commands class.
 */
template <typename T, typename S>
class LLA_READERS_ISO7816_API SAMISO7816Commands : public SAMCommands<T, S>
{
  public:
    /**
     * \brief Constructor.
     */
    SAMISO7816Commands()
        : SAMCommands<T, S>(CMD_SAMISO7816)
    {
        /*
        # Only one active MIFARE authentication at a time is supported by SAM AV2, so
        interleaved processing of the commands over differents LCs in parallel is not
        possible.
        d_named_mutex.reset(new
        boost::interprocess::named_mutex(boost::interprocess::open_or_create,
        "sam_mutex"));
        bool locked = d_named_mutex->timed_lock(boost::get_system_time() +
        boost::posix_time::seconds(5));

        boost::interprocess::shared_memory_object
        shm_obj(boost::interprocess::open_or_create, "sam_memory",
        boost::interprocess::read_write);
        boost::interprocess::offset_t size = 0;
        shm_obj.get_size(size);
        if (size != 4)
        shm_obj.truncate(4);

        d_region.reset(new boost::interprocess::mapped_region(shm_obj,
        boost::interprocess::read_write));

        char *addr = (char*)d_region->get_address();

        if (size != 4 || !locked)
        std::memset(addr, 0, d_region->get_size());

        unsigned char x = 0;
        for (; x < d_region->get_size(); ++x)
        {
        if (addr[x] == 0)
        {
        addr[x] = 1;
        break;
        }
        }

        d_named_mutex->unlock();

        if (x < d_region->get_size())
        d_cla = DEFAULT_SAM_CLA + x;
        else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "No channel available.");*/
        d_cla = DEFAULT_SAM_CLA;
        d_LastSessionIV.resize(16);
    }

    explicit SAMISO7816Commands(std::string ct)
        : SAMCommands<T, S>(ct)
    {
        d_cla = DEFAULT_SAM_CLA;
        d_LastSessionIV.resize(16);
    }

    /**
     * \brief Destructor.
     */
    virtual ~SAMISO7816Commands()
    {
        /*d_named_mutex->lock();
        char *addr = (char*)d_region->get_address();
        addr[d_cla - 0x80] = 0;
        d_named_mutex->unlock();

        if (!boost::interprocess::shared_memory_object::remove("sam_memory"))
        LOG(LogLevel::INFOS) << "SAM Shared Memory removed failed. It is probably still
        open by a process.");*/

        // we do not remove named_mutex - it can still be used by another process
    }

    std::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter()
    {
        return std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
            this->getReaderCardAdapter());
    }

    ByteVector transmit(ByteVector cmd, bool /*first*/ = true,
                        bool /*last*/ = true) override
    {
        return getISO7816ReaderCardAdapter()->sendCommand(cmd);
    }

    SAMVersion getVersion() override
    {
        unsigned char cmd[] = {d_cla, 0x60, 0x00, 0x00, 0x00};
        ByteVector cmd_vector(cmd, cmd + 5);
        SAMVersion info;
        memset(&info, 0x00, sizeof(SAMVersion));

        ByteVector result = transmit(cmd_vector);

        if (result.size() == 33 && result[31] == 0x90 && result[32] == 0x00)
            memcpy(&info, &result[0], sizeof(info));
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getVersion failed.");

        return info;
    }

    ByteVector decipherData(ByteVector data, bool islastdata) override
    {
        unsigned char p1 = 0x00;
        ByteVector datawithlength(3);

        if (!islastdata)
            p1 = 0xaf;
        else
        {
            datawithlength[0] = (unsigned char)(data.size() & 0xff0000);
            datawithlength[1] = (unsigned char)(data.size() & 0x00ff00);
            datawithlength[2] = (unsigned char)(data.size() & 0x0000ff);
        }
        datawithlength.insert(datawithlength.end(), data.begin(), data.end());

        unsigned char cmd[] = {
            d_cla, 0xdd, p1, 0x00, (unsigned char)(datawithlength.size()), 0x00};
        ByteVector cmd_vector(cmd, cmd + 6);
        cmd_vector.insert(cmd_vector.end() - 1, datawithlength.begin(),
                          datawithlength.end());

        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 && result[result.size() - 2] != 0x90 &&
            ((p1 == 0x00 && result[result.size() - 1] != 0x00) ||
             (p1 == 0xaf && result[result.size() - 1] != 0xaf)))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "decipherData failed.");

        return ByteVector(result.begin(), result.end() - 2);
    }

    ByteVector encipherData(ByteVector data, bool islastdata) override
    {
        unsigned char p1 = 0x00;

        if (!islastdata)
            p1              = 0xaf;
        unsigned char cmd[] = {d_cla, 0xed, p1, 0x00, (unsigned char)(data.size()), 0x00};
        ByteVector cmd_vector(cmd, cmd + 6);
        cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());
        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 && result[result.size() - 2] != 0x90 &&
            ((p1 == 0x00 && result[result.size() - 1] != 0x00) ||
             (p1 == 0xaf && result[result.size() - 1] != 0xaf)))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "encipherData failed.");

        return ByteVector(result.begin(), result.end() - 2);
    }

    void disableKeyEntry(unsigned char keyno) override
    {
        unsigned char cmd[] = {d_cla, 0xd8, keyno, 0x00};
        ByteVector cmd_vector(cmd, cmd + 4);

        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 &&
            (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "disableKeyEntry failed.");
    }

    ByteVector dumpSessionKey() override
    {
        unsigned char cmd[] = {d_cla, 0xd5, 0x00, 0x00, 0x00};
        ByteVector cmd_vector(cmd, cmd + 5);

        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 &&
            (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "dumpSessionKey failed.");

        return ByteVector(result.begin(), result.end() - 2);
    }

    void loadInitVector(ByteVector iv) override
    {
        EXCEPTION_ASSERT_WITH_LOG((iv.size() == 0x08 || iv.size() == 0x10),
                                  LibLogicalAccessException,
                                  "loadInitVector need a 16 or 8 bytes vector");

        ByteVector loadInitVector = {
            0x80, 0x71, 0x00, 0x00, static_cast<unsigned char>(iv.size()),
        };
        loadInitVector.insert(loadInitVector.end(), iv.begin(), iv.end());

        transmit(loadInitVector);
    }

    std::string getSAMTypeFromSAM() override
    {
        unsigned char cmd[] = {d_cla, 0x60, 0x00, 0x00, 0x00};
        ByteVector cmd_vector(cmd, cmd + 5);

        ByteVector result = transmit(cmd_vector);

        if (result.size() > 3)
        {
            if (result[result.size() - 3] == 0xA1)
                return "SAM_AV1";
            if (result[result.size() - 3] == 0xA2)
                return "SAM_AV2";
        }
        return "SAM_NONE";
    }

    virtual std::shared_ptr<SAMDESfireCrypto> getCrypto()
    {
        return d_crypto;
    }
    virtual void setCrypto(std::shared_ptr<SAMDESfireCrypto> t)
    {
        d_crypto = t;
    }

    void lockUnlock(std::shared_ptr<DESFireKey> masterKey, SAMLockUnlock state,
                    unsigned char keyno, unsigned char unlockkeyno,
                    unsigned char unlockkeyversion) override
    {
        unsigned char p1_part1 = state;
        unsigned int le        = 2;

        ByteVector maxChainBlocks(3, 0x00); // MaxChainBlocks - unlimited

        ByteVector data_p1(2, 0x00);
        data_p1[0] = keyno;
        data_p1[1] = masterKey->getKeyVersion();

        if (state == SwitchAV2Mode)
        {
            le += 3;
            data_p1.insert(data_p1.end(), maxChainBlocks.begin(), maxChainBlocks.end());
        }
        else if (state == LockWithSpecifyingKey)
        {
            le += 2;
            data_p1.push_back(unlockkeyno);
            data_p1.push_back(unlockkeyversion);
        }

        ByteVector result = this->getISO7816ReaderCardAdapter()->sendAPDUCommand(
            d_cla, 0x10, p1_part1, 0x00, le, data_p1, 0x00);
        if (result.size() != 14 || result[12] != 0x90 || result[13] != 0xAF)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "lockUnlock P1 Failed.");

        ByteVector keycipher(masterKey->getData(),
                             masterKey->getData() + masterKey->getLength());
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
        ByteVector emptyIV(16), rnd1;

        /* Create rnd2 for p3 - CMAC: rnd2 | P2 | other data */
        ByteVector rnd2(result.begin(), result.begin() + 12);
        rnd2.push_back(p1_part1);                                    // P1_part1
        rnd2.insert(rnd2.end(), data_p1.begin() + 2, data_p1.end()); // last data

        /* ZeroPad */
        if (state == LockWithSpecifyingKey)
        {
            rnd2.push_back(0x00);
        }
        else if (state != SwitchAV2Mode)
        {
            rnd2.resize(rnd2.size() + 3);
        }

        ByteVector macHost =
            openssl::CMACCrypto::cmac(keycipher, cipher, rnd2, emptyIV, 16);
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

        result = this->getISO7816ReaderCardAdapter()->sendAPDUCommand(
            d_cla, 0x10, 0x00, 0x00, 0x14, data_p2, 0x00);
        if (result.size() != 26 || result[24] != 0x90 || result[25] != 0xAF)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "lockUnlock P2 Failed.");

        /* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
        rnd1.insert(rnd1.end(), rnd2.begin() + 12, rnd2.end()); // p2 data without rnd2

        macHost = openssl::CMACCrypto::cmac(keycipher, cipher, rnd1, emptyIV, 16);
        truncateMacBuffer(macHost);

        for (unsigned char x = 0; x < 8; ++x)
        {
            if (macHost[x] != result[x])
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "lockUnlock P2 CMAC from SAM is Wrong.");
        }

        /* Create kxe - d_authkey */
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
                openssl::AESInitializationVector::createFromData(emptyIV)));

        ByteVector encRndB(result.begin() + 8, result.end() - 2);
        ByteVector dencRndB;

        cipher->decipher(encRndB, dencRndB, *symkey.get(), *iv.get(), false);

        // create rndB'
        ByteVector rndB1;
        rndB1.insert(rndB1.begin(), dencRndB.begin() + 2,
                     dencRndB.begin() + dencRndB.size());
        rndB1.push_back(dencRndB[0]);
        rndB1.push_back(dencRndB[1]);

        ByteVector dataHost, encHost;
        dataHost.insert(dataHost.end(), rndA.begin(), rndA.end());   // RndA
        dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); // RndB'

        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(emptyIV)));
        cipher->cipher(dataHost, encHost, *symkey.get(), *iv.get(), false);

        result = this->getISO7816ReaderCardAdapter()->sendAPDUCommand(
            d_cla, 0x10, 0x00, 0x00, 0x20, encHost, 0x00);
        if (result.size() != 18 || result[16] != 0x90 || result[17] != 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "lockUnlock P3 Failed.");

        ByteVector encSAMrndA(result.begin(), result.end() - 2), SAMrndA;
        iv.reset(new openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(emptyIV)));
        cipher->decipher(encSAMrndA, SAMrndA, *symkey.get(), *iv.get(), false);
        SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

        if (!equal(SAMrndA.begin(), SAMrndA.begin() + 16, rndA.begin()))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "lockUnlock P3 RndA from SAM is invalide.");
    }

    void selectApplication(ByteVector aid) override
    {
        unsigned char cmd[] = {d_cla, 0x5a, 0x00, 0x00, 0x03};
        ByteVector cmd_vector(cmd, cmd + 5);
        cmd_vector.insert(cmd_vector.end(), aid.begin(), aid.end());

        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 &&
            (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "selectApplication failed.");
    }

    ByteVector changeKeyPICC(const ChangeKeyInfo &info,
                             const ChangeKeyDiversification &diversifycation) override
    {
        unsigned char keyCompMeth = info.oldKeyInvolvement;

        unsigned char cfg = info.desfireNumber & 0xf;
        if (info.isMasterKey)
            cfg |= 0x10;
        ByteVector data(4);
        data[0] = info.currentKeySlotNo;
        data[1] = info.currentKeySlotV;
        data[2] = info.newKeySlotNo;
        data[3] = info.newKeySlotV;

        if (diversifycation.divType != NO_DIV)
        {
            if (diversifycation.divType == SAMAV2)
                keyCompMeth |= 0x20;

            keyCompMeth |= diversifycation.diversifyCurrent == 0x01 ? 0x04 : 0x00;
            keyCompMeth |= diversifycation.diversifyNew == 0x01 ? 0x02 : 0x00;

            data.insert(data.end(), diversifycation.divInput,
                        diversifycation.divInput + diversifycation.divInputSize);
        }

        unsigned char cmd[] = {
            d_cla, 0xc4, keyCompMeth, cfg, (unsigned char)(data.size()), 0x00};
        ByteVector cmd_vector(cmd, cmd + 6);
        cmd_vector.insert(cmd_vector.end() - 1, data.begin(), data.end());

        ByteVector result = transmit(cmd_vector);

        if (result.size() >= 2 &&
            (result[result.size() - 2] != 0x90 || result[result.size() - 1] != 0x00))
        {
            char tmp[64];
            sprintf(tmp, "changeKeyPICC failed (%x %x).", result[result.size() - 2],
                    result[result.size() - 1]);
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmp);
        }

        return ByteVector(result.begin(), result.end() - 2);
    }

  protected:
    std::shared_ptr<SAMDESfireCrypto> d_crypto;

    // std::shared_ptr<boost::interprocess::mapped_region> d_region;

    // std::shared_ptr<boost::interprocess::named_mutex> d_named_mutex;

    unsigned char d_cla;

    ByteVector d_authKey;

    ByteVector d_sessionKey;

    ByteVector d_LastSessionIV;

    static void truncateMacBuffer(ByteVector &data)
    {
        unsigned char truncateCount = 0;
        unsigned char count         = 1;

        while (count < data.size())
        {
            data[truncateCount] = data[count];
            count += 2;
            ++truncateCount;
        }
    }

    void generateAuthEncKey(ByteVector keycipher, ByteVector rnd1, ByteVector rnd2)
    {
        ByteVector SV1a(16), emptyIV(16);

        copy(rnd1.begin() + 7, rnd1.begin() + 12, SV1a.begin());
        copy(rnd2.begin() + 7, rnd2.begin() + 12, SV1a.begin() + 5);
        copy(rnd1.begin(), rnd1.begin() + 5, SV1a.begin() + 10);

        for (unsigned char x = 0; x <= 4; ++x)
        {
            SV1a[x + 10] ^= rnd2[x];
        }

        SV1a[15] = 0x91; /* AES 128 */
        /* TODO AES 192 */

        std::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(keycipher)));
        std::shared_ptr<openssl::InitializationVector> iv(
            new openssl::AESInitializationVector(
                openssl::AESInitializationVector::createFromData(emptyIV)));
        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

        cipher->cipher(SV1a, d_authKey, *symkey.get(), *iv.get(), false);
    }
};
}

#endif /* LOGICALACCESS_SAMAV1ISO7816COMMANDS_HPP */