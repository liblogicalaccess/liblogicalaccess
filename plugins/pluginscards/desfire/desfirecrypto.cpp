/**
 * \file desfirecrypto.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire cryptographic functions.
 */

#include "desfirecommands.hpp"
#include "desfirecrypto.hpp"
#include "desfireev1location.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <boost/crc.hpp>
#include <ctime>
#include <cstdlib>

#include <cstring>

#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "logicalaccess/crypto/cmac.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    DESFireCrypto::DESFireCrypto()
    {
        d_auth_method = CM_LEGACY;
        d_currentAid = 0;
        d_currentKeyNo = 0;
        d_mac_size = 4;
        d_block_size = 8;
        d_keys.clear();

        d_lastIV.clear();
        d_lastIV.resize(d_block_size, 0x00);
    }

    DESFireCrypto::~DESFireCrypto()
    {
    }

    void DESFireCrypto::appendDecipherData(const std::vector<unsigned char>& data)
    {
        d_buf.insert(d_buf.end(), data.begin(), data.end());
    }

	std::vector<unsigned char> DESFireCrypto::desfireDecrypt(size_t length)
    {
        std::vector<unsigned char> ret;

        if (d_auth_method == CM_LEGACY)
        {
            ret = desfire_decrypt(d_sessionKey, d_buf, length);
        }
        else
        {
            ret = desfire_iso_decrypt(d_sessionKey, d_buf, d_cipher, d_block_size, length);
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
            d_lastIV.resize(d_block_size, 0x00);
        }
    }

    bool DESFireCrypto::verifyMAC(bool end, const std::vector<unsigned char>& data)
    {
        bool ret = false;
        d_buf.insert(d_buf.end(), data.begin(), data.end());

        if (end)
        {
            if (d_auth_method == CM_LEGACY)	// Native DESFire mode
            {
                std::vector<unsigned char> mac;
                mac.insert(mac.end(), d_buf.end() - 4, d_buf.end());
                std::vector<unsigned char> ourMacBuf;
                ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 4);
                std::vector<unsigned char> ourMac = desfire_mac(d_sessionKey, ourMacBuf);
                ret = (mac == ourMac);
            }
            else
            {
                std::vector<unsigned char> mac;
                mac.insert(mac.end(), d_buf.end() - 8, d_buf.end());
                std::vector<unsigned char> ourMacBuf;
                ourMacBuf.insert(ourMacBuf.end(), d_buf.begin(), d_buf.end() - 8);
                ourMacBuf.push_back(0x00); // SW_OPERATION_OK
                std::vector<unsigned char> ourMac = desfire_cmac(d_sessionKey, d_cipher, d_block_size, ourMacBuf);
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

	std::vector<unsigned char> DESFireCrypto::generateMAC(unsigned char cmd, const std::vector<unsigned char>& data)
    {
        std::vector<unsigned char> ret;
        if (d_auth_method == CM_LEGACY)
        {
            ret = desfire_mac(d_sessionKey, data);
        }
        else
        {
            ret = desfire_cmac(d_sessionKey, d_cipher, d_block_size, data);
        }

        return ret;
    }

    std::vector<unsigned char> DESFireCrypto::desfireEncrypt(const std::vector<unsigned char>& data, const std::vector<unsigned char>& param, bool calccrc)
    {
        std::vector<unsigned char> ret;

        if (d_auth_method == CM_LEGACY)
        {
            ret = desfire_encrypt(d_sessionKey, data, calccrc);
        }
        else
        {
            ret = desfire_iso_encrypt(d_sessionKey, data, d_cipher, d_block_size, param, calccrc);
        }

        return ret;
    }

    short DESFireCrypto::desfire_crc16(const void* data, size_t dataLength)
    {
        unsigned char first, second;

        ComputeCrc(CRC_A, reinterpret_cast<const unsigned char*>(data), dataLength, &first, &second);

        return ((second << 8) | first);
    }

    uint32_t DESFireCrypto::desfire_crc32(const void* data, size_t dataLength)
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
    static bool is_triple_des(const std::vector<uint8_t> &key)
    {
        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException, "Triple des key check need a valid key size.");

        for (int i = 0; i < 8; ++i)
        {
            if ((key[i] & 0xFE) != (key[i + 8] & 0xFE))
                return true; // Bytes differ, so its triple DES.
        }
        return false;
    }

    std::vector<unsigned char> DESFireCrypto::desfire_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
    {
        symmetric_key skey;
        unsigned char in[8], out[8], in2[8];

        unsigned int i = 0;
        unsigned int j = 0;

        std::vector<unsigned char> ret;

        bool is3des = false;
        if (is_triple_des(key))
        {
            is3des = true;
        }

        // Set encryption keys
        if (is3des)
        {
            EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException, "DESFire send cbc encryption need a valid 3des key.");
            des3_setup(&key[0], 16, 0, &skey);
        }
        else
        {
            EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException, "DESFire send cbc encryption need a valid key.");
            des_setup(&key[0], 8, 0, &skey);
        }

        // clear buffers
        memset(in, 0x00, 8);
        memset(out, 0x00, 8);
        memset(in2, 0x00, 8);

        // do for each 8 byte block of input
        for (i = 0; i < data.size() / 8; i++)
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

    std::vector<unsigned char> DESFireCrypto::desfire_CBC_receive(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
    {
        symmetric_key skey;
        unsigned char in[8], out[8], in2[8];

        unsigned int i = 0;
        unsigned int j = 0;

        std::vector<unsigned char> ret;

        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException, "DESFire encryption need a valid key.");

        bool is3des = false;
        if (memcmp(&key[0], &key[8], 8))
        {
            is3des = true;
        }

        // Set encryption keys
        if (is3des)
        {
            EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException, "DESFire encryption need a valid 3des key.");
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
        for (i = 0; i < data.size() / 8; i++)
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

    std::vector<unsigned char> DESFireCrypto::desfire_CBC_mac(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
    {
        std::vector<unsigned char> ret = sam_CBC_send(key, iv, data);
        return std::vector<unsigned char>(ret.end() - 8, ret.end() - 4);
    }

    std::vector<unsigned char> DESFireCrypto::sam_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
    {
        unsigned char in[8], out[8];
        symmetric_key skey;
        unsigned int i = 0;
        unsigned int j = 0;

        std::vector<unsigned char> ret;

        EXCEPTION_ASSERT_WITH_LOG(key.size() >= 8, LibLogicalAccessException, "DESFire sam cbc encryption need a valid key.");

        bool is3des = false;
        if (memcmp(&key[0], &key[8], 8))
        {
            is3des = true;
        }

        // Set encryption keys
        if (is3des)
        {
            EXCEPTION_ASSERT_WITH_LOG(key.size() >= 16, LibLogicalAccessException, "DESFire sam cbc encryption need a valid key.");
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
        for (i = 0; i < data.size() / 8; i++)
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

    std::vector<unsigned char> DESFireCrypto::desfire_mac(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
    {
        int pad = (8 - (data.size() % 8)) % 8;
        for (int i = 0; i < pad; ++i)
        {
            data.push_back(0x00);
        }

        return desfire_CBC_mac(key, std::vector<unsigned char>(), data);
    }

    std::vector<unsigned char> DESFireCrypto::desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data, bool calccrc)
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
        return desfire_CBC_send(key, std::vector<unsigned char>(), data);
    }

    std::vector<unsigned char> DESFireCrypto::sam_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
    {
        int pad = (8 - ((data.size() + 2) % 8)) % 8;

        short crc = desfire_crc16(&data[0], data.size());
        data.push_back(static_cast<unsigned char>(crc & 0xff));
        data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
        for (int i = 0; i < pad; ++i)
        {
            data.push_back(0x00);
        }

        return sam_CBC_send(key, std::vector<unsigned char>(), data);
    }

	ByteVector DESFireCrypto::desfire_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, size_t datalen)
    {
        std::vector<unsigned char> ret;
        size_t ll = 0;

        ret = desfire_CBC_receive(key, std::vector<unsigned char>(), data);

        if (datalen == 0)
        {
            ll = ret.size() - 1;

            while (ret[ll] == 0x00)
            {
                ll--;
            }

            EXCEPTION_ASSERT_WITH_LOG(ret[ll] == 0x80, LibLogicalAccessException, "Incorrect FLT result");

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
        sprintf(computationError, "Error in crc computation: (crc1: %04x, crc2: %04x)", crc1, crc2);
        EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2, LibLogicalAccessException, computationError);

        ret.resize(ll);
        return ret;
    }

    std::vector<unsigned char> DESFireCrypto::authenticate_PICC1(unsigned char keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB)
    {
        d_sessionKey.clear();
        d_authkey.resize(16);
        getKey(d_currentAid, 0, keyno, diversify, d_authkey);
        d_rndB = desfire_CBC_send(d_authkey, std::vector<unsigned char>(), encRndB);

        std::vector<unsigned char> rndB1;
        rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 8);
        rndB1.push_back(d_rndB[0]);

        EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

        d_rndA.clear();
        d_rndA.resize(8);
        if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        std::vector<unsigned char> rndAB;
        rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
        rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

        return desfire_CBC_send(d_authkey, std::vector<unsigned char>(), rndAB);
    }

    void DESFireCrypto::authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
    {
        std::vector<unsigned char> rndA = desfire_CBC_send(d_authkey, std::vector<unsigned char>(), encRndA1);
        std::vector<unsigned char> checkRndA;

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
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4, d_rndA.begin() + 8);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4, d_rndB.begin() + 8);
            }

            d_currentKeyNo = keyno;
        }
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFire authentication failed - rnda are not the same.");

        d_auth_method = CM_LEGACY;
        d_mac_size = 4;
    }

    void DESFireCrypto::getKey(std::shared_ptr<DESFireKey> key, std::vector<unsigned char> diversify, std::vector<unsigned char>& keydiv)
    {
        LOG(LogLevel::INFOS) << "Init key from crypto with diversify set to: " << BufferHelper::getHex(diversify) << ".";

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
                keydiv.insert(keydiv.end(), key->getData(), key->getData() + key->getLength());
            }
        }
        if (key->getKeyType() != DF_KEY_AES)
            getKeyVersioned(key, keydiv);
    }

    void DESFireCrypto::getKeyVersioned(std::shared_ptr<DESFireKey> key, std::vector<unsigned char>& keyversioned)
    {
        unsigned char* keytmpversioned = new unsigned char[key->getLength()];
        memset(keytmpversioned, 0x00, key->getLength());
        unsigned char version = key->getKeyVersion();
        // set the key version
        for (unsigned char i = 0; i < 8; i++)
        {
            keytmpversioned[7 - i] = (unsigned char)((unsigned char)(keyversioned[7 - i] & 0xFE) | (unsigned char)((version >> i) & 0x01));
            keytmpversioned[15 - i] = (unsigned char)(keyversioned[15 - i]);
            if (key->getLength() == 24)
            {
                keytmpversioned[23 - i] = (unsigned char)(keyversioned[23 - i]);
            }
        }
        keyversioned.clear();
        keyversioned.insert(keyversioned.end(), keytmpversioned, keytmpversioned + key->getLength());
        delete[] keytmpversioned;
    }

    void DESFireCrypto::selectApplication(size_t aid)
    {
        d_currentAid = static_cast<int>(aid);

        d_auth_method = CM_LEGACY;
        d_cipher.reset();
        d_currentKeyNo = 0;
        d_sessionKey.clear();
    }

    std::vector<unsigned char> DESFireCrypto::changeKey_PICC(uint8_t keyno, std::shared_ptr<DESFireKey> newkey, std::vector<unsigned char> diversify, unsigned char keysetno)
    {
        LOG(LogLevel::INFOS) << "Init change key on PICC...";
        std::vector<unsigned char> cryptogram;
        std::vector<unsigned char> oldkeydiv, newkeydiv;
        oldkeydiv.resize(16, 0x00);
        newkeydiv.resize(16, 0x00);
        getKey(keysetno, keyno, diversify, oldkeydiv);
        getKey(newkey, diversify, newkeydiv);

        std::vector<unsigned char> encCryptogram;

        if (d_auth_method == CM_LEGACY) // Native DESFire
        {
            if (keyno != d_currentKeyNo || keysetno)
            {
	            for (unsigned int i = 0; i < newkeydiv.size(); ++i)
                {
                    encCryptogram.push_back(static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
                }

				if (keysetno != 0)
					encCryptogram.push_back(keysetno); //ChangeKeyEV2
                short crc = desfire_crc16(&encCryptogram[0], encCryptogram.size());
                encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
                crc = desfire_crc16(&newkeydiv[0], newkeydiv.size());
                encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
                // Pad
                for (int i = 0; i < 4; ++i)
                {
                    encCryptogram.push_back(0x00);
                }
                cryptogram = desfire_CBC_send(d_sessionKey, std::vector<unsigned char>(), encCryptogram);
            }
            else
            {
                cryptogram = desfire_encrypt(d_sessionKey, newkeydiv);
            }
        }
        else
        {
            if (keyno != d_currentKeyNo)
            {
                uint32_t crc;
                for (unsigned int i = 0; i < newkeydiv.size(); ++i)
                {
                    encCryptogram.push_back(static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
                }

                std::shared_ptr<openssl::SymmetricKey> sessionkey;
                std::shared_ptr<openssl::InitializationVector> iv;

                if (std::dynamic_pointer_cast<openssl::AESCipher>(d_cipher))
                {
                    // For AES, add key version.
                    encCryptogram.push_back(newkey->getKeyVersion());

                    sessionkey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_sessionKey)));
                    iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
                }
                else
                {
                    sessionkey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(d_sessionKey)));
                    iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
                }
                std::vector<unsigned char> crcoldkxor;
                crcoldkxor.push_back(DF_INS_CHANGE_KEY);
                crcoldkxor.push_back(keyno);
                crcoldkxor.insert(crcoldkxor.end(), encCryptogram.begin(), encCryptogram.end());
                crc = desfire_crc32(&crcoldkxor[0], crcoldkxor.size());
                encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
                crc = desfire_crc32(&newkeydiv[0], newkeydiv.size());
                encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
                encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));

                int pad = (d_block_size - (encCryptogram.size() % d_block_size)) % d_block_size;
                for (int i = 0; i < pad; ++i)
                {
                    encCryptogram.push_back(0x00);
                }

                d_cipher->cipher(encCryptogram, cryptogram, *sessionkey, *iv, false);
                d_lastIV = std::vector<unsigned char>(cryptogram.end() - d_block_size, cryptogram.end());
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

    std::vector<unsigned char> DESFireCrypto::iso_authenticate_PICC1(unsigned char keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB, unsigned int randomlen)
    {
        d_sessionKey.clear();
        getKey(d_currentAid, 0, keyno, diversify, d_authkey);
        d_cipher.reset(new openssl::DESCipher());
        openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
        openssl::DESInitializationVector iv = openssl::DESInitializationVector::createNull();
        d_rndB.clear();
        d_cipher->decipher(encRndB, d_rndB, deskey, iv, false);
        d_lastIV = std::vector<unsigned char>(encRndB.end() - randomlen, encRndB.end());

        std::vector<unsigned char> rndB1;
        rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + randomlen);
        rndB1.push_back(d_rndB[0]);

        EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

        d_rndA.clear();
        d_rndA.resize(randomlen);
        if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        std::vector<unsigned char> rndAB;
        rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
        rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

        std::vector<unsigned char> ret;
		iv = openssl::DESInitializationVector::createFromData(d_lastIV);
        d_cipher->cipher(rndAB, ret, deskey, iv, false);
        d_lastIV = std::vector<unsigned char>(ret.end() - randomlen, ret.end());

        return ret;
    }

    void DESFireCrypto::iso_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1, unsigned int randomlen)
    {
        std::vector<unsigned char> checkRndA;
        std::vector<unsigned char> rndA;
        openssl::DESSymmetricKey deskey = openssl::DESSymmetricKey::createFromData(d_authkey);
        openssl::DESInitializationVector iv = openssl::DESInitializationVector::createFromData(d_lastIV);
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
                    d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4, d_rndA.begin() + 8);
                    d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4, d_rndB.begin() + 8);
                }
            }
            // 3K3DES
            else if (d_authkey.size() == 24)
            {
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 6, d_rndA.begin() + 10);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 6, d_rndB.begin() + 10);
                d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
                d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);
            }

            d_currentKeyNo = keyno;
        }
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFire authentication failed - rnda are not the same.");

        d_cipher.reset(new openssl::DESCipher());
        d_auth_method = CM_ISO;
        d_block_size = 8;
        d_mac_size = 8;
        d_lastIV.clear();
        d_lastIV.resize(d_block_size, 0x00);
    }

    std::vector<unsigned char> DESFireCrypto::aes_authenticate_PICC1(unsigned char keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB)
    {
        d_sessionKey.clear();
        getKey(d_currentAid, 0, keyno, diversify, d_authkey);
        d_cipher.reset(new openssl::AESCipher());
        openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
        openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
        d_rndB.clear();
        d_cipher->decipher(encRndB, d_rndB, aeskey, iv, false);
        d_lastIV = std::vector<unsigned char>(encRndB.end() - 16, encRndB.end());

        std::vector<unsigned char> rndB1;
        rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
        rndB1.push_back(d_rndB[0]);

        EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

        d_rndA.clear();
        d_rndA.resize(16);
        if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
        }

        std::vector<unsigned char> rndAB;
        rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
        rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

        std::vector<unsigned char> ret;
		iv = openssl::AESInitializationVector::createFromData(d_lastIV);
		d_cipher->cipher(rndAB, ret, aeskey, iv, false);
        d_lastIV = std::vector<unsigned char>(ret.end() - 16, ret.end());

        return ret;
    }

    void DESFireCrypto::aes_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
    {
        std::vector<unsigned char> checkRndA;
        std::vector<unsigned char> rndA;
        openssl::AESSymmetricKey aeskey = openssl::AESSymmetricKey::createFromData(d_authkey);
        openssl::AESInitializationVector iv = openssl::AESInitializationVector::createFromData(d_lastIV);
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
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "AES Authenticate PICC 2 Failed!");

        d_cipher.reset(new openssl::AESCipher());
        d_auth_method = CM_ISO;
        d_block_size = 16;
        d_mac_size = 8;
        d_lastIV.clear();
        d_lastIV.resize(d_block_size, 0x00);
    }

    std::vector<unsigned char> DESFireCrypto::desfire_cmac(const std::vector<unsigned char>& data)
    {
        return desfire_cmac(d_sessionKey, d_cipher, d_block_size, data);
    }

    std::vector<unsigned char> DESFireCrypto::desfire_cmac(const std::vector<unsigned char>& key, std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data)
    {
        std::vector<unsigned char> ret = openssl::CMACCrypto::cmac(key, cipherMAC, block_size, data, d_lastIV, block_size);

        if (cipherMAC == d_cipher)
        {
            d_lastIV = std::vector<unsigned char>(ret.end() - block_size, ret.end());
        }

        // DES
        if (block_size == 8)
        {
            ret = std::vector<unsigned char>(ret.end() - 8, ret.end());
        }
        else
        {
            ret = std::vector<unsigned char>(ret.end() - 16, ret.end() - 8);
        }

        return ret;
    }

    std::vector<unsigned char> DESFireCrypto::desfire_iso_decrypt(const std::vector<unsigned char>& data, size_t length)
    {
        return desfire_iso_decrypt(d_sessionKey, data, d_cipher, d_block_size, length);
    }

    std::vector<unsigned char> DESFireCrypto::desfire_iso_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher, unsigned int block_size, size_t datalen)
    {
        std::vector<unsigned char> decdata;
        std::shared_ptr<openssl::SymmetricKey> isokey;
        std::shared_ptr<openssl::InitializationVector> iv;

        if (std::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
        }

        assert(cipher);
        cipher->decipher(data, decdata, *isokey, *iv, false);
        if (cipher == d_cipher)
        {
            d_lastIV = std::vector<unsigned char>(data.end() - block_size, data.end());
        }

        size_t ll = 0;

        if (datalen == 0)
        {
			// Lets analyse the padding and find crc ourself
            ll = decdata.size() - 1;

			while (ll > 0 && decdata[ll] == 0x00)
            {
                ll--;
            }

            EXCEPTION_ASSERT_WITH_LOG(decdata[ll] == 0x80, LibLogicalAccessException, "Incorrect FLT result");

			decdata[ll] = 0x00; // Remove 0x80 for padding check

            ll -= 4; // Move to crc start

			EXCEPTION_ASSERT_WITH_LOG(ll >= 0, LibLogicalAccessException, "Cannot find the crc in the encrypted data");
        }
        else
        {
            ll = datalen;
        }

        std::vector<unsigned char> crcbuf = std::vector<unsigned char>(decdata.begin(), decdata.begin() + ll);
        crcbuf.push_back(0x00);	// SW_OPERATION_OK
        uint32_t crc1 = desfire_crc32(&crcbuf[0], crcbuf.size());
        uint32_t crc2 = decdata[ll] | (decdata[ll + 1] << 8) | (decdata[ll + 2] << 16) | (decdata[ll + 3] << 24);
        size_t pad = decdata.size() - ll - 4;
        std::vector<unsigned char> padding = std::vector<unsigned char>(decdata.begin() + ll + 4, decdata.begin() + ll + 4 + pad);
        std::vector<unsigned char> padding1;
        for (size_t i = 0; i < pad; ++i)
        {
            padding1.push_back(0x00);
        }
        std::stringstream ss;
        ss << "Error in crc computation: (crc1: " << std::hex << crc1;
        ss << ", crc2: " << crc2 << ") or padding. Padding: " << padding << ". padding1: " << padding1;
        EXCEPTION_ASSERT_WITH_LOG(crc1 == crc2 && padding == padding1, LibLogicalAccessException, ss.str());

        decdata.resize(ll);
        return decdata;
    }

    std::vector<unsigned char> DESFireCrypto::iso_encipherData(bool end, const std::vector<unsigned char>& data, const std::vector<unsigned char>& param)
    {
        std::vector<unsigned char> encdata;
        d_buf.insert(d_buf.end(), data.begin(), data.end());

        std::vector<unsigned char> decdata = data;
        decdata.insert(decdata.begin(), d_last_left.begin(), d_last_left.end());

        if (end) {
            std::vector<unsigned char> calconbuf = param;
            calconbuf.insert(calconbuf.end(), d_buf.begin(), d_buf.end());
            uint32_t crc = desfire_crc32(&calconbuf[0], calconbuf.size());
            decdata.push_back(static_cast<unsigned char>(crc & 0xff));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
            int pad = (d_block_size - (decdata.size() % d_block_size)) % d_block_size;
            for (int i = 0; i < pad; ++i)
            {
                decdata.push_back(0x00);
            }
        }
        else {
            int leave = decdata.size() % d_block_size;
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
                isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_sessionKey)));
                iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
            }
            else
            {
                isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(d_sessionKey)));
                iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
            }
            d_cipher->cipher(decdata, encdata, *isokey, *iv, false);
            d_lastIV = std::vector<unsigned char>(encdata.end() - d_block_size, encdata.end());
        }

        return encdata;
    }

    std::vector<unsigned char> DESFireCrypto::desfire_iso_encrypt(const std::vector<unsigned char>& key,
                                                                  const std::vector<unsigned char>& data,
                                                                  std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher,
                                                                  unsigned int block_size,
                                                                  const std::vector<unsigned char>& param,
                                                                  bool calccrc)
    {
        std::vector<unsigned char> encdata;
        std::vector<unsigned char> decdata = data;
        std::vector<unsigned char> calconbuf = param;
        calconbuf.insert(calconbuf.end(), data.begin(), data.end());
        if (calccrc)
        {
            uint32_t crc = desfire_crc32(&calconbuf[0], calconbuf.size());
            decdata.push_back(static_cast<unsigned char>(crc & 0xff));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
            decdata.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));
        }
        int pad = (block_size - (decdata.size() % block_size)) % block_size;
        for (int i = 0; i < pad; ++i)
        {
            decdata.push_back(0x00);
        }

        std::shared_ptr<openssl::SymmetricKey> isokey;
        std::shared_ptr<openssl::InitializationVector> iv;
        if (std::dynamic_pointer_cast<openssl::AESCipher>(cipher))
        {
            isokey.reset(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_lastIV)));
        }
        else
        {
            isokey.reset(new openssl::DESSymmetricKey(openssl::DESSymmetricKey::createFromData(key)));
            iv.reset(new openssl::DESInitializationVector(openssl::DESInitializationVector::createFromData(d_lastIV)));
        }
        cipher->cipher(decdata, encdata, *isokey, *iv, false);
        if (cipher == d_cipher)
        {
            d_lastIV = std::vector<unsigned char>(encdata.end() - block_size, encdata.end());
        }

        return encdata;
    }

    void DESFireCrypto::setCryptoContext(std::vector<unsigned char> identifier)
    {
        d_identifier = identifier;
		clearKeys();
    }

    std::shared_ptr<DESFireKey> DESFireCrypto::getKey(uint8_t keyslot, uint8_t keyno) const
    {
        return getKey(d_currentAid, keyslot, keyno);
    }

    void DESFireCrypto::createApplication(int aid, uint8_t keyslotNb, uint8_t maxNbKeys, DESFireKeyType cryptoMethod)
    {
		for (unsigned char j = 0; j < keyslotNb; ++j)
		{
			for (unsigned char i = 0; i < maxNbKeys; ++i)
			{
				setKey(aid, j, i, getDefaultKey(cryptoMethod));
			}
		}
    }

	void DESFireCrypto::setDefaultKeysAt(std::shared_ptr<Location> location)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<DESFireEV1Location> dfEV1Location = std::dynamic_pointer_cast<DESFireEV1Location>(location);
		std::shared_ptr<DESFireLocation> dfLocation = std::dynamic_pointer_cast<DESFireLocation>(location);

		if (dfLocation)
			LOG(LogLevel::WARNINGS) << "DESFireEV1Profile use DESFireLocation so we force DES Crypto.";
		else if (!dfEV1Location)
			EXCEPTION_ASSERT_WITH_LOG(dfEV1Location, std::invalid_argument, "location must be a DESFireEV1Location or DESFireLocation.");

		// Application (File keys are Application keys)
		if (dfLocation->aid != (unsigned int)-1)
		{
			for (unsigned char i = 0; i < 14; ++i)
			{
				if (dfEV1Location)
					setKey(dfEV1Location->aid, 0, i, getDefaultKey(dfEV1Location->cryptoMethod));
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
		std::vector<unsigned char> buf;
		buf.resize(key->getLength(), 0x00);
		key->setData(buf);

		return key;
	}

	std::shared_ptr<DESFireKey> DESFireCrypto::getKey(size_t aid, uint8_t keyslot, uint8_t keyno) const
	{
		std::shared_ptr<DESFireKey> key;

		auto it = d_keys.find(std::make_tuple(aid, keyslot, keyno));
		if (it != d_keys.end())
			key = it->second;
		else
			key = DESFireCrypto::getDefaultKey(DF_KEY_DES);

		return key;
	}

	bool DESFireCrypto::getKey(uint8_t keyslot, uint8_t keyno, std::vector<unsigned char> diversify, std::vector<unsigned char>& keydiv)
	{
		return getKey(d_currentAid, keyslot, keyno, diversify, keydiv);
	}

	bool DESFireCrypto::getKey(size_t aid, uint8_t keyslot, uint8_t keyno, std::vector<unsigned char> diversify, std::vector<unsigned char>& keydiv)
	{
		auto it = d_keys.find(std::make_tuple(aid, keyslot, keyno));
		if (it == d_keys.end())
			return false;

		DESFireCrypto::getKey(it->second, diversify, keydiv);

		return true;
	}

	void DESFireCrypto::setKey(size_t aid, uint8_t keyslot, uint8_t keyno, std::shared_ptr<DESFireKey> key)
	{
		d_keys[std::make_tuple(aid, keyslot, keyno)] = key;
	}

	void DESFireCrypto::setKeyInAllKeySet(size_t aid, uint8_t nbKeySlots, uint8_t nbKeys, std::shared_ptr<DESFireKey> key)
	{
		for (auto j = 0; j < nbKeySlots; ++j)
			for (auto x = 0; x < nbKeys; ++x)
				d_keys[std::make_tuple(aid, j, x)] = key;
	}
}
