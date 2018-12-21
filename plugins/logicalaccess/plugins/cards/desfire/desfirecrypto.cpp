/**
 * \file desfirecrypto.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire cryptographic functions.
 */

#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/plugins/crypto/tomcrypt.h>
#include <boost/crc.hpp>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <openssl/rand.h>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/des_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/des_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/iks/IslogKeyServer.hpp>
#include <logicalaccess/iks/RemoteCrypto.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/services/aes_crypto_service.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>

namespace logicalaccess
{
DESFireCrypto::DESFireCrypto()
{
    d_auth_method  = CM_LEGACY;
    d_currentAid   = 0;
    d_currentKeyNo = 0;
    d_mac_size     = 4;
    d_keys.clear();

    d_lastIV.clear();
    d_lastIV.resize(8, 0x00);
}

DESFireCrypto::~DESFireCrypto()
{
}

void DESFireCrypto::appendDecipherData(const ByteVector &data)
{
    d_buf.insert(d_buf.end(), data.begin(), data.end());
}

ByteVector DESFireCrypto::desfireDecrypt(size_t length)
{
    ByteVector ret;

    if (d_auth_method == CM_LEGACY)
    {
        ret = desfire_decrypt(d_sessionKey, d_buf, length);
    }
    else
    {
        ret = desfire_iso_decrypt(d_sessionKey, d_buf, d_cipher, length);
    }

    return ret;
}

void DESFireCrypto::initBuf()
{
    d_buf.clear();
    d_last_left.clear();
    if (d_auth_method == CM_LEGACY)
    {
        d_lastIV.clear();
        if (d_cipher)
		{
            d_lastIV.resize(d_cipher->getBlockSize(), 0x00);
		}
    }
}

bool DESFireCrypto::verifyMAC(bool end, const ByteVector &data)
{
    bool ret;
    d_buf.insert(d_buf.end(), data.begin(), data.end());

    if (end)
    {
        if (d_auth_method == CM_LEGACY) // Native DESFire mode
        {
            EXCEPTION_ASSERT_WITH_LOG(d_buf.size() >= 4, LibLogicalAccessException,
                                      "Wrong MAC buffer length.");

            ByteVector mac;
            mac.insert(mac.end(), d_buf.end() - 4, d_buf.end());
            ByteVector ourMacBuf;
            ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 4);
            ByteVector ourMac = desfire_mac(d_sessionKey, ourMacBuf);
            ret               = (mac == ourMac);
        }
        else
        {
            EXCEPTION_ASSERT_WITH_LOG(d_buf.size() >= 8, LibLogicalAccessException,
                                      "Wrong MAC buffer length.");

            ByteVector mac;
            mac.insert(mac.end(), d_buf.end() - 8, d_buf.end());
            ByteVector ourMacBuf;
            ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 8);
            ourMacBuf.push_back(0x00); // SW_OPERATION_OK
            ByteVector ourMac =
                desfire_cmac(d_sessionKey, d_cipher, ourMacBuf);
            ret = (mac == ourMac);
        }

        d_buf.clear();
    }
    else
    {
        ret = true;
    }

    return ret;
}

ByteVector DESFireCrypto::generateMAC(unsigned char /*cmd*/, const ByteVector &data)
{
    if (!data.size())
        return {};

    ByteVector ret;
    if (d_auth_method == CM_LEGACY)
    {
        ret = desfire_mac(d_sessionKey, data);
    }
    else
    {
        ret = desfire_cmac(d_sessionKey, d_cipher, data);
    }

    return ret;
}

ByteVector DESFireCrypto::desfireEncrypt(const ByteVector &data, const ByteVector &param,
                                         bool calccrc)
{
    ByteVector ret;

    if (d_auth_method == CM_LEGACY)
    {
        ret = desfire_encrypt(d_sessionKey, data, calccrc);
    }
    else
    {
        ret = desfire_iso_encrypt(d_sessionKey, data, d_cipher, param,
                                  calccrc);
    }

    return ret;
}

short DESFireCrypto::desfire_crc16(const void *data, size_t dataLength)
{
    unsigned char first, second;

    ComputeCrc(CRC_A, reinterpret_cast<const unsigned char *>(data), dataLength, &first,
               &second);

    return ((second << 8) | first);
}

uint32_t DESFireCrypto::desfire_crc32(const void *data, size_t dataLength)
{
    boost::crc_32_type result;
    result.process_bytes(data, dataLength);
    uint32_t crc = result.checksum();
    crc ^= 0xffffffff;
    crc = (-1 * (crc ^ 0xffffffff)) - 1;
    return crc;
}

/**
 * Check wheter or not a key is triple DES.
 * We do this by comparing the 8 first byte to 8 second byte, ignoring
 * parity bits.
 */
static bool is_triple_des(const ByteVector &key)
{
    EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException,
                              "Triple des key check need a valid key size.");

    for (int i = 0; i < 8; ++i)
    {
        if ((key[i] & 0xFE) != (key[i + 8] & 0xFE))
            return true; // Bytes differ, so its triple DES.
    }
    return false;
}

ByteVector DESFireCrypto::desfire_CBC_send(const ByteVector &key, const ByteVector &iv,
                                           const ByteVector &data)
{
    symmetric_key skey;
    unsigned char in[8], out[8], in2[8];

    unsigned int j;

    ByteVector ret;

    bool is3des = false;
    if (is_triple_des(key))
    {
        is3des = true;
    }

    // Set encryption keys
    if (is3des)
    {
        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException,
                                  "DESFire send cbc encryption need a valid 3des key.");
        des3_setup(&key[0], 16, 0, &skey);
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException,
                                  "DESFire send cbc encryption need a valid key.");
        des_setup(&key[0], 8, 0, &skey);
    }

    // clear buffers
    memset(in, 0x00, 8);
    memset(out, 0x00, 8);
    memset(in2, 0x00, 8);

    // do for each 8 byte block of input
    for (unsigned int i = 0; i < data.size() / 8; i++)
    {
        // copy 8 bytes from input buffer to in
        memcpy(in, &data[i * 8], 8);

        if (i == 0)
        {
            if (iv.size() >= 8)
            {
                for (j = 0; j < 8; j++)
                {
                    in[j] ^= iv[j];
                }
            }
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                in[j] ^= in2[j];
            }
        }

        // encryption
        if (is3des)
        {
            des3_ecb_decrypt(in, out, &skey);
        }
        else
        {
            des_ecb_decrypt(in, out, &skey);
        }

        memcpy(in2, out, 8);

        // copy decrypted block to output
        ret.insert(ret.end(), out, out + 8);
    }

    if (is3des)
    {
        des3_done(&skey);
    }
    else
    {
        des_done(&skey);
    }

    return ret;
}

ByteVector DESFireCrypto::desfire_CBC_receive(const ByteVector &key, const ByteVector &iv,
                                              const ByteVector &data)
{
    symmetric_key skey;
    unsigned char in[8], out[8], in2[8];

    unsigned int j;

    ByteVector ret;

    EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException,
                              "DESFire encryption need a valid key.");

    bool is3des = false;
    if (memcmp(&key[0], &key[8], 8))
    {
        is3des = true;
    }

    // Set encryption keys
    if (is3des)
    {
        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException,
                                  "DESFire encryption need a valid 3des key.");
        des3_setup(&key[0], 16, 0, &skey);
    }
    else
    {
        des_setup(&key[0], 8, 0, &skey);
    }

    // clear buffers
    memset(in, 0x00, 8);
    memset(out, 0x00, 8);
    memset(in2, 0x00, 8);

    // do for each 8 byte block of input
    for (unsigned int i = 0; i < data.size() / 8; i++)
    {
        // copy 8 bytes from input buffer to in
        memcpy(in, &data[i * 8], 8);

        if (is3des)
        {
            des3_ecb_decrypt(in, out, &skey);
        }
        else
        {
            des_ecb_decrypt(in, out, &skey);
        }

        if (i == 0)
        {
            if (iv.size() >= 8)
            {
                for (j = 0; j < 8; j++)
                {
                    out[j] ^= iv[j];
                }
            }
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                out[j] ^= in2[j];
            }
        }

        memcpy(in2, &data[i * 8], 8);

        // copy decrypted block to output
        ret.insert(ret.end(), out, out + 8);
    }

    if (is3des)
    {
        des3_done(&skey);
    }
    else
    {
        des_done(&skey);
    }

    return ret;
}

ByteVector DESFireCrypto::desfire_CBC_mac(const ByteVector &key, const ByteVector &iv,
                                          const ByteVector &data)
{
    ByteVector ret = sam_CBC_send(key, iv, data);
    return ByteVector(ret.end() - 8, ret.end() - 4);
}

ByteVector DESFireCrypto::sam_CBC_send(const ByteVector &key, const ByteVector &iv,
                                       const ByteVector &data)
{
    unsigned char in[8], out[8];
    symmetric_key skey;
    unsigned int j;

    ByteVector ret;

    EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException,
                              "DESFire sam cbc encryption need a valid key.");

    bool is3des = false;
    if (memcmp(&key[0], &key[8], 8))
    {
        is3des = true;
    }

    // Set encryption keys
    if (is3des)
    {
        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException,
                                  "DESFire sam cbc encryption need a valid key.");
        des3_setup(&key[0], 16, 0, &skey);
    }
    else
    {
        des_setup(&key[0], 8, 0, &skey);
    }

    // clear buffers
    memset(in, 0x00, 8);
    memset(out, 0x00, 8);

    // do for each 8 byte block of input
    for (unsigned int i = 0; i < data.size() / 8; i++)
    {
        // copy 8 bytes from input buffer to in
        memcpy(in, &data[i * 8], 8);

        if (i == 0)
        {
            if (iv.size() >= 8)
            {
                for (j = 0; j < 8; j++)
                {
                    in[j] ^= iv[j];
                }
            }
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                in[j] = in[j] ^ out[j];
            }
        }

        if (is3des)
        {
            des3_ecb_encrypt(in, out, &skey);
        }
        else
        {
            des_ecb_encrypt(in, out, &skey);
        }

        // copy encrypted block to output
        ret.insert(ret.end(), out, out + 8);
    }

    if (is3des)
    {
        des3_done(&skey);
    }
    else
    {
        des_done(&skey);
    }

    return ret;
}

ByteVector DESFireCrypto::desfire_mac(const ByteVector &key, ByteVector data)
{
    int pad = (8 - (data.size() % 8)) % 8;
    for (int i = 0; i < pad; ++i)
    {
        data.push_back(0x00);
    }

    return desfire_CBC_mac(key, ByteVector(), data);
}

ByteVector DESFireCrypto::desfire_encrypt(const ByteVector &key, ByteVector data,
                                          bool calccrc)
{
    if (calccrc)
    {
        short crc = desfire_crc16(&data[0], data.size());
        data.push_back(static_cast<unsigned char>(crc & 0xff));
        data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
    }
    int pad = (8 - (data.size() % 8)) % 8;
    for (int i = 0; i < pad; ++i)
    {
        data.push_back(0x00);
    }
    return desfire_CBC_send(key, ByteVector(), data);
}

ByteVector DESFireCrypto::sam_encrypt(const ByteVector &key, ByteVector data)
{
    int pad = (8 - ((data.size() + 2) % 8)) % 8;

    short crc = desfire_crc16(&data[0], data.size());
    data.push_back(static_cast<unsigned char>(crc & 0xff));
    data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
    for (int i = 0; i < pad; ++i)
    {
        data.push_back(0x00);
    }

    return sam_CBC_send(key, ByteVector(), data);
}

ByteVector DESFireCrypto::desfire_decrypt(const ByteVector &key, const ByteVector &data,
                                          size_t datalen)
{
    ByteVector ret;
    size_t ll;

    ret = desfire_CBC_receive(key, ByteVector(), data);

    if (datalen == 0)
    {
        ll = ret.size() - 1;

        while (ret[ll] == 0x00)
        {
            ll--;
        }

        EXCEPTION_ASSERT_WITH_LOG(ret[ll] == 0x80, LibLogicalAccessException,
                                  "Incorrect FLT result");

        ll -= 2;
    }
    else
    {
        ll = datalen;
    }

    unsigned short crc1 = desfire_crc16(&ret[0], ll);
    unsigned short crc2 = static_cast<unsigned short>(ret[ll] | (ret[ll + 1] << 8));

    char computationError[128];
    memset(computationError, 0x00, 128);
    sprintf(computationError, "Error in crc computation: (crc1: %04x, crc2: %04x)", crc1,
            crc2);
    EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2, LibLogicalAccessException, computationError);

    ret.resize(ll);
    return ret;
}

ByteVector DESFireCrypto::authenticate_PICC1(unsigned char keyno, ByteVector diversify,
                                             const ByteVector &encRndB)
{
    d_sessionKey.clear();
    d_authkey.resize(16);
    getKey(d_currentAid, 0, keyno, diversify, d_authkey);
    d_rndB = desfire_CBC_send(d_authkey, ByteVector(), encRndB);

    ByteVector rndB1;
    rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 8);
    rndB1.push_back(d_rndB[0]);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");

    d_rndA.clear();
    d_rndA.resize(8);
    if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector rndAB;
    rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
    rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

    return desfire_CBC_send(d_authkey, ByteVector(), rndAB);
}

void DESFireCrypto::authenticate_PICC2(unsigned char keyno, const ByteVector &encRndA1)
{
    ByteVector rndA = desfire_CBC_send(d_authkey, ByteVector(), encRndA1);
    ByteVector checkRndA;

    d_sessionKey.clear();
    checkRndA.push_back(rndA[7]);
    checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 7);

    if (d_rndA == checkRndA)
    {
        // DES
        if (!memcmp(&d_authkey[0], &d_authkey[8], 8))
        {
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
        }
        // 3DES
        else
        {
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
            d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4,
                                d_rndA.begin() + 8);
            d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4,
                                d_rndB.begin() + 8);
        }

        d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "DESFire authentication failed - rnda are not the same.");

    d_auth_method = CM_LEGACY;
    d_mac_size    = 4;
}

void DESFireCrypto::getKey(std::shared_ptr<DESFireKey> key, ByteVector diversify,
                           ByteVector &keydiv)
{
    LOG(LogLevel::INFOS) << "Init key from crypto with diversify set to: "
                         << BufferHelper::getHex(diversify) << ".";

    keydiv.clear();
    if (key->getKeyDiversification() && diversify.size() != 0)
    {
        LOG(LogLevel::INFOS) << "Use key diversification.";
        keydiv = key->getKeyDiversification()->getDiversifiedKey(key, diversify);
    }
    else
    {
        if (key->isEmpty())
        {
            keydiv.resize(key->getLength(), 0x00);
        }
        else
        {
            keydiv.insert(keydiv.end(), key->getData(),
                          key->getData() + key->getLength());
        }
    }
    if (key->getKeyType() != DF_KEY_AES)
        getKeyVersioned(key, keydiv);
}

void DESFireCrypto::getKeyVersioned(std::shared_ptr<DESFireKey> key,
                                    ByteVector &keyversioned)
{
    auto keytmpversioned  = key->getBytes();
    unsigned char version = key->getKeyVersion();
    // set the key version
    for (unsigned char i = 0; i < 8; i++)
    {
        keytmpversioned[7 - i] =
            (unsigned char)((unsigned char)(keyversioned[7 - i] & 0xFE) |
                            (unsigned char)((version >> i) & 0x01));
        keytmpversioned[15 - i] = (unsigned char)(keyversioned[15 - i]);
        if (key->getLength() == 24)
        {
            keytmpversioned[23 - i] = (unsigned char)(keyversioned[23 - i]);
        }
    }
    keyversioned = keytmpversioned;
}

void DESFireCrypto::selectApplication(size_t aid)
{
    d_currentAid = static_cast<int>(aid);

    d_auth_method = CM_LEGACY;
    d_cipher.reset();
    d_currentKeyNo = 0;
    d_sessionKey.clear();
}

ByteVector DESFireCrypto::changeKey_PICC(uint8_t keyno, ByteVector oldKeyDiversify,
                                         std::shared_ptr<DESFireKey> newkey,
                                         ByteVector newKeyDiversify,
                                         unsigned char keysetno)
{
    LOG(LogLevel::INFOS) << "Init change key on PICC...";
    ByteVector cryptogram;
    ByteVector oldkeydiv, newkeydiv;
    oldkeydiv.resize(16, 0x00);
    newkeydiv.resize(16, 0x00);
    // Get keyno only, in case of master card key
    unsigned char keyno_only = static_cast<unsigned char>(keyno & 0x3F);
    getKey(keysetno, keyno_only, oldKeyDiversify, oldkeydiv);
    getKey(newkey, newKeyDiversify, newkeydiv);

    ByteVector encCryptogram;

    if (d_auth_method == CM_LEGACY) // Native DESFire
    {
        if (keyno_only != d_currentKeyNo || keysetno)
        {
            for (unsigned int i = 0; i < newkeydiv.size(); ++i)
            {
                encCryptogram.push_back(
                    static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
            }

            if (keysetno != 0 && newkey->getKeyType() == DF_KEY_AES)
                encCryptogram.push_back(newkey->getKeyVersion()); // ChangeKeyEV2
            short crc = desfire_crc16(&encCryptogram[0], encCryptogram.size());
            encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            crc = desfire_crc16(&newkeydiv[0], newkeydiv.size());
            encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            encCryptogram.resize(24); // Pad
            cryptogram = desfire_CBC_send(d_sessionKey, ByteVector(), encCryptogram);
        }
        else
        {
            if (newkey->getKeyType() == DF_KEY_AES) // Change PICC Key
                newkeydiv.push_back(newkey->getKeyVersion());
            cryptogram = desfire_encrypt(d_sessionKey, newkeydiv);
        }
    }
    else
    {
        if (keyno_only != d_currentKeyNo || keysetno)
        {
            for (unsigned int i = 0; i < newkeydiv.size(); ++i)
            {
                encCryptogram.push_back(
                    static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
            }

            std::shared_ptr<openssl::SymmetricKey> sessionkey;
            std::shared_ptr<openssl::InitializationVector> iv;

            if (std::dynamic_pointer_cast<openssl::AESCipher>(d_cipher))
            {
                // For AES, add key version.
                encCryptogram.push_back(newkey->getKeyVersion());

                sessionkey.reset(new openssl::AESSymmetricKey(
                    openssl::AESSymmetricKey::createFromData(d_sessionKey)));
                iv.reset(new openssl::AESInitializationVector(
                    openssl::AESInitializationVector::createFromData(d_lastIV)));
            }
            else
            {
                sessionkey.reset(new openssl::DESSymmetricKey(
                    openssl::DESSymmetricKey::createFromData(d_sessionKey)));
                iv.reset(new openssl::DESInitializationVector(
                    openssl::DESInitializationVector::createFromData(d_lastIV)));
            }
            auto crcoldkxor = ByteVector({DF_INS_CHANGE_KEY, keyno});
            if (keysetno != 0) // ChangeKeyEV2
            {
                crcoldkxor[0] = 0xc6; // DFEV2_CHANGEKEYEV2
                crcoldkxor.insert(crcoldkxor.begin() + 1, keysetno);
            }
            crcoldkxor.insert(crcoldkxor.end(), encCryptogram.begin(),
                              encCryptogram.end());
            uint32_t crc = desfire_crc32(&crcoldkxor[0], crcoldkxor.size());
            encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
            crc = desfire_crc32(&newkeydiv[0], newkeydiv.size());
            encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
            encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));

            int pad = (d_cipher->getBlockSize() -
                       (encCryptogram.size() % d_cipher->getBlockSize())) %
                      d_cipher->getBlockSize();
            for (int i = 0; i < pad; ++i)
            {
                encCryptogram.push_back(0x00);
            }

            d_cipher->cipher(encCryptogram, cryptogram, *sessionkey, *iv, false);
            d_lastIV =
                ByteVector(cryptogram.end() - d_cipher->getBlockSize(), cryptogram.end());
        }
        else
        {
            if (newkey->getKeyType() == DF_KEY_AES)
            {
                // For AES, add key version.
                newkeydiv.push_back(newkey->getKeyVersion());
            }

            encCryptogram.push_back(DF_INS_CHANGE_KEY);
            encCryptogram.push_back(keyno);
            cryptogram = desfireEncrypt(newkeydiv, encCryptogram);
        }
    }

    return cryptogram;
}

ByteVector DESFireCrypto::iso_authenticate_PICC1(unsigned char keyno,
                                                 ByteVector diversify,
                                                 const ByteVector &encRndB,
                                                 unsigned int randomlen)
{
    d_sessionKey.clear();
    getKey(d_currentAid, 0, keyno, diversify, d_authkey);
    d_cipher.reset(new openssl::DESCipher());
    openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
    openssl::DESInitializationVector iv = openssl::DESInitializationVector::createNull();
    d_rndB.clear();
    d_cipher->decipher(encRndB, d_rndB, deskey, iv, false);
    d_lastIV = ByteVector(encRndB.end() - randomlen, encRndB.end());

    ByteVector rndB1;
    rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + randomlen);
    rndB1.push_back(d_rndB[0]);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");

    d_rndA.clear();
    d_rndA.resize(randomlen);
    if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    ByteVector rndAB;
    rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
    rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

    ByteVector ret;
    iv = openssl::DESInitializationVector::createFromData(d_lastIV);
    d_cipher->cipher(rndAB, ret, deskey, iv, false);
    d_lastIV = ByteVector(ret.end() - randomlen, ret.end());

    return ret;
}

void DESFireCrypto::iso_authenticate_PICC2(unsigned char keyno,
                                           const ByteVector &encRndA1,
                                           unsigned int randomlen)
{
    ByteVector checkRndA;
    ByteVector rndA;
    openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
    openssl::DESInitializationVector iv =
        openssl::DESInitializationVector::createFromData(d_lastIV);
    d_cipher->decipher(encRndA1, rndA, deskey, iv, false);

    d_sessionKey.clear();
    checkRndA.push_back(rndA[randomlen - 1]);
    checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + randomlen - 1);

    if (d_rndA == checkRndA)
    {
        // 2K3DES
        if (d_authkey.size() == 16)
        {
            // DES
            if (!memcmp(&d_authkey[0], &d_authkey[8], 8))
            {
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(),
                                    d_rndA.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(),
                                    d_rndB.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(),
                                    d_rndA.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(),
                                    d_rndB.begin() + 4);
            }
            // 3DES
            else
            {
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(),
                                    d_rndA.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(),
                                    d_rndB.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4,
                                    d_rndA.begin() + 8);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4,
                                    d_rndB.begin() + 8);
            }
        }
        // 3K3DES
        else if (d_authkey.size() == 24)
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
        }

        d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "DESFire authentication failed - rnda are not the same.");

    d_cipher.reset(new openssl::DESCipher());
    d_auth_method = CM_ISO;
    d_mac_size    = 8;
    d_lastIV.clear();
    d_lastIV.resize(d_cipher->getBlockSize(), 0x00);
}

ByteVector DESFireCrypto::aes_authenticate_PICC1(unsigned char keyno,
                                                 ByteVector diversify,
                                                 const ByteVector &encRndB)
{
    d_sessionKey.clear();
    getKey(d_currentAid, 0, keyno, diversify, d_authkey);
    d_cipher.reset(new openssl::AESCipher());
    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
    openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
    d_rndB.clear();
    d_cipher->decipher(encRndB, d_rndB, aeskey, iv, false);
    d_lastIV = ByteVector(encRndB.end() - 16, encRndB.end());

    ByteVector rndB1;
    rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
    rndB1.push_back(d_rndB[0]);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");

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
    iv = openssl::AESInitializationVector::createFromData(d_lastIV);
    d_cipher->cipher(rndAB, ret, aeskey, iv, false);
    d_lastIV = ByteVector(ret.end() - 16, ret.end());

    return ret;
}

ByteVector DESFireCrypto::aes_authenticate_PICC1_GENERIC(unsigned char keyno,
                                                         const std::shared_ptr<Key> &key,
                                                         const ByteVector &encRndB)
{
    d_sessionKey.clear();

    AESCryptoService aes_crypto;
    d_rndB   = aes_crypto.aes_decrypt(encRndB, {}, key);
    d_lastIV = ByteVector(encRndB.end() - 16, encRndB.end());

    ByteVector rndB1;
    rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
    rndB1.push_back(d_rndB[0]);

    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient enthropy source");

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

    ByteVector ret = aes_crypto.aes_encrypt(rndAB, d_lastIV, key);
    d_lastIV       = ByteVector(ret.end() - 16, ret.end());
    return ret;
}

void DESFireCrypto::aes_authenticate_PICC2(unsigned char keyno,
                                           const ByteVector &encRndA1)
{
    ByteVector checkRndA;
    ByteVector rndA;
    openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
    openssl::AESInitializationVector iv =
        openssl::AESInitializationVector::createFromData(d_lastIV);
    d_cipher->decipher(encRndA1, rndA, aeskey, iv, false);

    d_sessionKey.clear();
    checkRndA.push_back(rndA[15]);
    checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 15);

    if (d_rndA == checkRndA)
    {
        d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
        d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
        d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
        d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);

        d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "AES Authenticate PICC 2 Failed!");
}

void DESFireCrypto::aes_authenticate_PICC2_GENERIC(unsigned char keyno,
                                                   const std::shared_ptr<Key> &key,
                                                   const ByteVector &encRndA1)
{
    ByteVector checkRndA;
    ByteVector rndA;
    AESCryptoService aes_crypto;
    rndA = aes_crypto.aes_decrypt(encRndA1, d_lastIV, key);

    d_sessionKey.clear();
    checkRndA.push_back(rndA[15]);
    checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 15);

    if (d_rndA == checkRndA)
    {
        d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
        d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
        d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
        d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);

        d_currentKeyNo = keyno;
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "AES Authenticate PICC 2 Failed!");
}

ByteVector DESFireCrypto::desfire_cmac(const ByteVector &data)
{
    return desfire_cmac(d_sessionKey, d_cipher, data);
}

ByteVector
DESFireCrypto::desfire_cmac(const ByteVector &key,
                            std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, const ByteVector &data)
{

    if (iks_wrapper_)
    {
        auto remote_crypto = LibraryManager::getInstance()->getRemoteCrypto();
        ByteVector ret     = openssl::CMACCrypto::cmac_iks(iks_wrapper_->remote_key_name, data, d_lastIV,
                                          cipherMAC->getBlockSize(), remote_crypto);
        d_lastIV = ByteVector(ret.end() - cipherMAC->getBlockSize(), ret.end());
        // Need to understand this "if (chipher == d_cipher)".
        ret = ByteVector(ret.end() - 16, ret.end() - 8);
        return ret;
    }

    ByteVector ret = openssl::CMACCrypto::cmac(key, cipherMAC, data, d_lastIV,
                                               cipherMAC->getBlockSize());

    if (cipherMAC == d_cipher)
    {
        d_lastIV = ByteVector(ret.end() - cipherMAC->getBlockSize(), ret.end());
    }

    // DES
    if (cipherMAC->getBlockSize() == 8)
    {
        ret = ByteVector(ret.end() - 8, ret.end());
    }
    else
    {
        ret = ByteVector(ret.end() - 16, ret.end() - 8);
    }

    return ret;
}

ByteVector DESFireCrypto::desfire_iso_decrypt(const ByteVector &data, size_t length)
{
    return desfire_iso_decrypt(d_sessionKey, data, d_cipher, length);
}

ByteVector DESFireCrypto::desfire_iso_decrypt(
    const ByteVector &key, const ByteVector &data,
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher,
    size_t datalen)
{
    std::vector<uint8_t> decdata;

    if (iks_wrapper_ == nullptr)
    {
        std::shared_ptr<openssl::SymmetricKey> isokey;
        std::shared_ptr<openssl::InitializationVector> iv;
        if (std::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(
                openssl::AESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::AESInitializationVector(
                openssl::AESInitializationVector::createFromData(d_lastIV)));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(
                openssl::DESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::DESInitializationVector(
                openssl::DESInitializationVector::createFromData(d_lastIV)));
        }

        assert(cipher);
        cipher->decipher(data, decdata, *isokey, *iv, false);
        if (cipher == d_cipher)
        {
            d_lastIV = ByteVector(data.end() - cipher->getBlockSize(), data.end());
        }
    }
    else
    {
        // Delegate cryptography to the IKS.
        auto remote_crypto = LibraryManager::getInstance()->getRemoteCrypto();
        SignatureResult signature_result;
        decdata = remote_crypto->aes_decrypt(data, iks_wrapper_->remote_key_name,
                                             d_lastIV, &signature_result);
        d_lastIV = ByteVector(data.end() - cipher->getBlockSize(), data.end());
        iks_wrapper_->last_sig = signature_result;
    }
    size_t ll;

    if (datalen == 0)
    {
        // Lets analyse the padding and find crc ourself
        ll = decdata.size() - 1;

        while (ll > 0 && decdata[ll] == 0x00)
        {
            ll--;
        }

        EXCEPTION_ASSERT_WITH_LOG(decdata[ll] == 0x80, LibLogicalAccessException,
                                  "Incorrect FLT result");

        decdata[ll] = 0x00; // Remove 0x80 for padding check

        EXCEPTION_ASSERT_WITH_LOG(ll >= 4, LibLogicalAccessException,
                                  "Cannot find the crc in the encrypted data");

        ll -= 4; // Move to crc start
    }
    else
    {
        ll = datalen;
    }

    ByteVector crcbuf = ByteVector(decdata.begin(), decdata.begin() + ll);
    crcbuf.push_back(0x00); // SW_OPERATION_OK
    uint32_t crc1 = desfire_crc32(&crcbuf[0], crcbuf.size());
    uint32_t crc2 = decdata[ll] | (decdata[ll + 1] << 8) | (decdata[ll + 2] << 16) |
                    (decdata[ll + 3] << 24);
    size_t pad = decdata.size() - ll - 4;
    ByteVector padding =
        ByteVector(decdata.begin() + ll + 4, decdata.begin() + ll + 4 + pad);
    ByteVector padding1;
    for (size_t i = 0; i < pad; ++i)
    {
        padding1.push_back(0x00);
    }
    std::stringstream ss;
    ss << "Error in crc computation: (crc1: " << std::hex << crc1;
    ss << ", crc2: " << crc2 << ") or padding. Padding: " << padding
       << ". padding1: " << padding1;
    EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2 && padding == padding1,
                              LibLogicalAccessException, ss.str());

    decdata.resize(ll);
    return decdata;
}

ByteVector DESFireCrypto::iso_encipherData(bool end, const ByteVector &data,
                                           const ByteVector &param)
{
    ByteVector encdata;
    d_buf.insert(d_buf.end(), data.begin(), data.end());

    ByteVector decdata = data;
    decdata.insert(decdata.begin(), d_last_left.begin(), d_last_left.end());

    if (end)
    {
        ByteVector calconbuf = param;
        calconbuf.insert(calconbuf.end(), d_buf.begin(), d_buf.end());
        uint32_t crc = desfire_crc32(&calconbuf[0], calconbuf.size());
        decdata.push_back(static_cast<unsigned char>(crc & 0xff));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
        int pad =
            (d_cipher->getBlockSize() - (decdata.size() % d_cipher->getBlockSize())) %
                  d_cipher->getBlockSize();
        for (int i = 0; i < pad; ++i)
        {
            decdata.push_back(0x00);
        }
    }
    else
    {
        int leave = decdata.size() % d_cipher->getBlockSize();
        if (leave > 0)
        {
            d_last_left.clear();
            d_last_left.insert(d_last_left.end(), decdata.end() - leave, decdata.end());
            decdata.resize(decdata.size() - leave);
        }
        else
        {
            d_last_left.clear();
        }
    }

    if (decdata.size() > 0)
    {
        std::shared_ptr<openssl::SymmetricKey> isokey;
        std::shared_ptr<openssl::InitializationVector> iv;
        if (std::dynamic_pointer_cast<openssl::AESCipher>(d_cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(
                openssl::AESSymmetricKey::createFromData(d_sessionKey)));
            iv.reset(new openssl::AESInitializationVector(
                openssl::AESInitializationVector::createFromData(d_lastIV)));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(
                openssl::DESSymmetricKey::createFromData(d_sessionKey)));
            iv.reset(new openssl::DESInitializationVector(
                openssl::DESInitializationVector::createFromData(d_lastIV)));
        }
        d_cipher->cipher(decdata, encdata, *isokey, *iv, false);
        d_lastIV = ByteVector(encdata.end() - d_cipher->getBlockSize(), encdata.end());
    }

    return encdata;
}

ByteVector DESFireCrypto::desfire_iso_encrypt(
    const ByteVector &key, const ByteVector &data,
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher,
    const ByteVector &param, bool calccrc)
{
    ByteVector encdata;
    ByteVector decdata   = data;
    ByteVector calconbuf = param;
    calconbuf.insert(calconbuf.end(), data.begin(), data.end());
    if (calccrc)
    {
        uint32_t crc = desfire_crc32(&calconbuf[0], calconbuf.size());
        decdata.push_back(static_cast<unsigned char>(crc & 0xff));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
        decdata.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
    }
    int pad = (cipher->getBlockSize() - (decdata.size() % cipher->getBlockSize())) %
              cipher->getBlockSize();
    for (int i = 0; i < pad; ++i)
    {
        decdata.push_back(0x00);
    }

    if (iks_wrapper_ == nullptr)
    {
        std::shared_ptr<openssl::SymmetricKey> isokey;
        std::shared_ptr<openssl::InitializationVector> iv;
        if (std::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(
                openssl::AESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::AESInitializationVector(
                openssl::AESInitializationVector::createFromData(d_lastIV)));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(
                openssl::DESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::DESInitializationVector(
                openssl::DESInitializationVector::createFromData(d_lastIV)));
        }
        cipher->cipher(decdata, encdata, *isokey, *iv, false);
        if (cipher == d_cipher)
        {
            d_lastIV = ByteVector(encdata.end() - cipher->getBlockSize(), encdata.end());
        }
    }
    else
    {
        // Delegate cryptography to the IKS.
        auto remote_crypto = LibraryManager::getInstance()->getRemoteCrypto();
        encdata =
            remote_crypto->aes_encrypt(decdata, iks_wrapper_->remote_key_name, d_lastIV);
        d_lastIV = ByteVector(encdata.end() - cipher->getBlockSize(), encdata.end());
    }

    return encdata;
}

void DESFireCrypto::setCryptoContext(ByteVector identifier)
{
    d_identifier = identifier;
    clearKeys();
}

std::shared_ptr<DESFireKey> DESFireCrypto::getKey(uint8_t keyset, uint8_t keyno) const
{
    return getKey(d_currentAid, keyset, keyno);
}

void DESFireCrypto::createApplication(int aid, uint8_t maxKeySetNb, uint8_t maxNbKeys,
                                      DESFireKeyType cryptoMethod)
{
    for (auto x = 0; x < maxKeySetNb; ++x)
        setKeyInAllKeySet(aid, x, maxNbKeys, getDefaultKey(cryptoMethod));
}

void DESFireCrypto::setDefaultKeysAt(std::shared_ptr<Location> location)
{
    EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument,
                              "location cannot be null.");

    std::shared_ptr<DESFireEV1Location> dfEV1Location =
        std::dynamic_pointer_cast<DESFireEV1Location>(location);
    std::shared_ptr<DESFireLocation> dfLocation =
        std::dynamic_pointer_cast<DESFireLocation>(location);

    if (dfLocation)
        LOG(LogLevel::WARNINGS)
            << "DESFireEV1Profile use DESFireLocation so we force DES Crypto.";
    else if (!dfEV1Location)
        EXCEPTION_ASSERT_WITH_LOG(
            dfEV1Location, std::invalid_argument,
            "location must be a DESFireEV1Location or DESFireLocation.");

    // Application (File keys are Application keys)
    if (dfLocation->aid != (unsigned int)-1)
    {
        for (unsigned char i = 0; i < 14; ++i)
        {
            if (dfEV1Location)
                setKey(dfEV1Location->aid, 0, i,
                       getDefaultKey(dfEV1Location->cryptoMethod));
            else
                setKey(dfLocation->aid, 0, i, getDefaultKey(DF_KEY_DES));
        }
    }
    // Card
    else
    {
        setKey(0, 0, 0, getDefaultKey(DF_KEY_DES));
    }
}

void DESFireCrypto::clearKeys()
{
    d_keys.clear();
}

std::shared_ptr<DESFireKey> DESFireCrypto::getDefaultKey(DESFireKeyType keyType)
{
    std::shared_ptr<DESFireKey> key(new DESFireKey());

    key->setKeyType(keyType);
    ByteVector buf;
    buf.resize(key->getLength(), 0x00);
    key->setData(buf);

    return key;
}

std::shared_ptr<DESFireKey> DESFireCrypto::getKey(size_t aid, uint8_t keyslot,
                                                  uint8_t keyno) const
{
    std::shared_ptr<DESFireKey> key;

    auto it = d_keys.find(std::make_tuple(aid, keyslot, keyno));
    if (it != d_keys.end())
        key = it->second;
    else
        key = getDefaultKey(DF_KEY_DES);

    return key;
}

bool DESFireCrypto::getKey(uint8_t keyslot, uint8_t keyno, ByteVector diversify,
                           ByteVector &keydiv)
{
    return getKey(d_currentAid, keyslot, keyno, diversify, keydiv);
}

bool DESFireCrypto::getKey(size_t aid, uint8_t keyslot, uint8_t keyno,
                           ByteVector diversify, ByteVector &keydiv)
{
    auto it = d_keys.find(std::make_tuple(aid, keyslot, keyno));
    if (it == d_keys.end())
        return false;

    getKey(it->second, diversify, keydiv);

    return true;
}

void DESFireCrypto::setKey(size_t aid, uint8_t keyslot, uint8_t keyno,
                           std::shared_ptr<DESFireKey> key)
{
    d_keys[std::make_tuple(aid, keyslot, keyno)] = std::make_shared<DESFireKey>(*key);
}

void DESFireCrypto::setKeyInAllKeySet(size_t aid, uint8_t keySlotNb, uint8_t nbKeys,
                                      std::shared_ptr<DESFireKey> key)
{
    for (auto x = 0; x < nbKeys; ++x)
        d_keys[std::make_tuple(aid, keySlotNb, x)] = key;
}

SignatureResult DESFireCrypto::get_last_signature() const
{
    if (iks_wrapper_)
        return iks_wrapper_->last_sig;

    return SignatureResult{};
}
}
