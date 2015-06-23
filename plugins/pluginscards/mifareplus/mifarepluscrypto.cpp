/**
 * \file mifarepluscrypto.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus cryptographic functions.
 */

#include "mifarepluscrypto.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <ctime>
#include <cstdlib>

#include <cstring>

#include <openssl/rand.h>
#include "logicalaccess/crypto/cmac.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    MifarePlusCrypto::MifarePlusCrypto()
    {
        d_Wctr = 0x00;
        d_Rctr = 0x00;
        d_ti.resize(4, 0x00);
    }

    MifarePlusCrypto::~MifarePlusCrypto()
    {
    }

    std::vector<unsigned char> MifarePlusCrypto::GetMacOnCommand(std::vector<unsigned char> command)
    {
        std::vector<unsigned char> data;
        std::vector<unsigned char> mac;
        int i;
        //determine whether command is a reading one or a writting one
        if (command[0] >= 0x30 && command[0] <= 0x37) //read command
        {
            data.resize(3);
            data[0] = command[0];
            data[1] = (char)d_Rctr;
            data[2] = (char)(d_Rctr >> 8);
            data.insert(data.end(), d_ti.begin(), d_ti.end());
            data.resize(10);
            data[7] = command[1];
            data[8] = command[2];
            data[9] = command[3];
        }
        else if ((command[0] >= 0xA0 && command[0] <= 0xA3) || (command[0] >= 0xB0 && command[0] <= 0xB9) || command[0] == 0xC2 || command[0] == 0xC3)  //write command
        {
            std::vector<unsigned char> tmp = command;

            data.resize(3);
            data[0] = command[0];
            data[1] = (char)d_Wctr;
            data[2] = (char)(d_Wctr >> 8);
            data.insert(data.end(), d_ti.begin(), d_ti.end());
            data.resize(9);
            data[7] = command[1];
            data[8] = command[2];
            tmp = std::vector<unsigned char>(tmp.begin() + 3, tmp.end());
            if (command[0] >= 0xB6 && command[0] <= 0xB9)
            {
                tmp = std::vector<unsigned char>(tmp.begin() + 2, tmp.end());
            }
            if (command[0] == 0xA0 || command[0] == 0xA1 || (command[0] >= 0xB0 && command[0] <= 0xB3) || (command[0] >= 0xB6 && command[0] <= 0xB9))
            {
                std::vector<unsigned char> tmpddec = AESUncipher(tmp, d_Kenc, false);
                data.insert(data.end(), tmpddec.begin(), tmpddec.end());
            }
            else
            {
                data.insert(data.end(), tmp.begin(), tmp.end());
            }
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The command is not recognized by the MACing process");
        //fonction conforme aux specifications NIST 800-38B
        std::vector<unsigned char> iv;
        iv.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data = openssl::CMACCrypto::cmac(d_Kmac, std::shared_ptr<openssl::OpenSSLSymmetricCipher>(new openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC)), MIFARE_PLUS_BLOCK_SIZE, data, iv, MIFARE_PLUS_BLOCK_SIZE);
        while (data.size() > MIFARE_PLUS_BLOCK_SIZE)
        {
            data = std::vector<unsigned char>(data.begin() + MIFARE_PLUS_BLOCK_SIZE, data.end());
        }

        //get 8 bytes of the mac
        mac.resize(8);
        for (i = 0; i < 8; i++)
        {
            mac[i] = data[i * 2 + 1];
        }

        return mac;
    }

    std::vector<unsigned char> MifarePlusCrypto::GetMacOnResponse(std::vector<unsigned char> response, std::vector<unsigned char> command)
    {
        std::vector<unsigned char> data;
        std::vector<unsigned char> mac;
        int i;
        //determine whether command is a reading one or a writting one
        if (command[0] >= (char)0x30 && command[0] <= (char)0x37) //read command
        {
            std::vector<unsigned char> tmp = response;

            data.resize(3);
            data[0] = response[0];
            data[1] = (char)d_Rctr;
            data[2] = (char)(d_Rctr >> 8);
            data.insert(data.end(), d_ti.begin(), d_ti.end());
            data.resize(10);
            data[7] = command[1];
            data[8] = command[2];
            data[9] = command[3];
            tmp = std::vector<unsigned char>(tmp.begin() + 1, tmp.end());
            if (tmp.size() > 8)
            {
                tmp.resize(8);
            }

            if (command[0] == 0x30 || command[0] == 0x31 || command[0] == 0x34 || command[0] == 0x35)
            {
                std::vector<unsigned char> tmpdec = AESUncipher(tmp, d_Kenc, false);
                data.insert(data.end(), tmpdec.begin(), tmpdec.end());
            }
            else
            {
                data.insert(data.end(), tmp.begin(), tmp.end());
            }
        }
        else if ((command[0] >= 0xA0 && command[0] <= 0xA3) || (command[0] >= 0xB0 && command[0] <= 0xB9) || command[0] == 0xC2 || command[0] == 0xC3) //write command
        {
            data.resize(3);
            data[0] = response[0];
            data[1] = (char)d_Wctr;
            data[2] = (char)(d_Wctr >> 8);
            data.insert(data.end(), d_ti.begin(), d_ti.end());
        }
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The command is not recognized by the MACing process");
        //fonction conforme aux specifications NIST 800-38B
        std::vector<unsigned char> iv;
        iv.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data = openssl::CMACCrypto::cmac(d_Kmac, std::shared_ptr<openssl::OpenSSLSymmetricCipher>(new openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC)), MIFARE_PLUS_BLOCK_SIZE, data, iv, MIFARE_PLUS_BLOCK_SIZE);
        while (data.size() > MIFARE_PLUS_BLOCK_SIZE)
        {
            data = std::vector<unsigned char>(data.begin() + MIFARE_PLUS_BLOCK_SIZE, data.end());
        }

        //get 8 bytes of the mac
        mac.resize(8);
        for (i = 0; i < 8; i++)
        {
            mac[i] = data[i * 2 + 1];
        }

        return mac;
    }

    void MifarePlusCrypto::calculateKmac()
    {
        std::vector<unsigned char> tmp;
        unsigned char f[5];
        unsigned char g[5];
        unsigned char h[5];
        unsigned char i[5];
        int c = 0;
        //init data buffers
        while (c < 5)
        {
            h[c] = d_rndA[c];
            i[c] = d_rndB[c];
            f[c] = d_rndA[c + 7];
            g[c] = d_rndB[c + 7];
            c++;
        }
        //set the session key base for Kmac
        tmp.insert(tmp.end(), f, f + 5);
        tmp.insert(tmp.end(), g, g + 5);
        tmp.resize(MIFARE_PLUS_AES_KEY_SIZE);
        c = 0;
        while (c < 5)
        {
            tmp[c + 10] = (h[c] ^ i[c]);
            c++;
        }
        tmp[15] = (char)0x22;

        //calculate Kmac by AES encryption
        d_Kmac = AESCipher(tmp, d_AESauthkey, false);
    }

    void MifarePlusCrypto::calculateKenc()
    {
        std::vector<unsigned char> tmp;
        unsigned char f[5];
        unsigned char g[5];
        unsigned char h[5];
        unsigned char i[5];
        int c = 0;
        //init data buffers
        while (c < 5)
        {
            h[c] = d_rndA[c + 4];
            i[c] = d_rndB[c + 4];
            f[c] = d_rndA[c + 11];
            g[c] = d_rndB[c + 11];
            c++;
        }
        //set the session key base for Kmac
        tmp.insert(tmp.end(), f, f + 5);
        tmp.insert(tmp.end(), g, g + 5);
        tmp.resize(MIFARE_PLUS_AES_KEY_SIZE);
        c = 0;
        while (c < 5)
        {
            tmp[c + 10] = (h[c] ^ i[c]);
            c++;
        }
        tmp[15] = (char)0x11;
        //calculate Kmac by AES encryption
        d_Kenc = AESCipher(tmp, d_AESauthkey, false);
    }

    std::vector<unsigned char> MifarePlusCrypto::AESUncipher(std::vector<unsigned char> in, std::vector<unsigned char> keyString, bool ivSL3)
    {
        std::vector<unsigned char> ivString;
        if (ivSL3)
        {
            std::vector<unsigned char> ivStringCountPart = GetIvStringCountPart();
            ivString.insert(ivString.end(), ivStringCountPart.begin(), ivStringCountPart.end());
            ivString.insert(ivString.end(), d_ti.begin(), d_ti.end());
        }
        else
        {
            ivString.resize(16, 0x00);
        }

        openssl::AESCipher *cipherUtils = new openssl::AESCipher();
        openssl::AESSymmetricKey aesKey = openssl::AESSymmetricKey::createFromData(keyString);
        openssl::AESInitializationVector iv = openssl::AESInitializationVector::createFromData(ivString);
        std::vector<unsigned char> out;

        //		cipherUtils->ENC_MODE_CBC;
        cipherUtils->decipher(in, out, aesKey, iv, false);

        delete cipherUtils;

        return (out);
    }

    std::vector<unsigned char> MifarePlusCrypto::AESCipher(std::vector<unsigned char> in, std::vector<unsigned char> keyString, bool ivSL3)
    {
        std::vector<unsigned char> ivString;
        if (ivSL3)
        {
            ivString.insert(ivString.end(), d_ti.begin(), d_ti.end());
            std::vector<unsigned char> ivStringCountPart = GetIvStringCountPart();
            ivString.insert(ivString.end(), ivStringCountPart.begin(), ivStringCountPart.end());
        }
        else
            ivString.resize(16, 0x00);

        openssl::AESCipher *cipherUtils = new openssl::AESCipher();
        openssl::AESSymmetricKey aesKey = openssl::AESSymmetricKey::createFromData(keyString);
        openssl::AESInitializationVector iv = openssl::AESInitializationVector::createFromData(ivString);
        std::vector<unsigned char> out;

        //		cipherUtils->ENC_MODE_CBC;
        cipherUtils->cipher(in, out, aesKey, iv, false);

        delete cipherUtils;

        return (out);
    }

    std::vector<unsigned char> MifarePlusCrypto::GetIvStringCountPart() const
    {
        std::vector<unsigned char> ivStringC;
        ivStringC.resize(12);

        ivStringC[0] = (char)d_Rctr;
        ivStringC[1] = (char)(d_Rctr >> 8);
        ivStringC[2] = (char)d_Wctr;
        ivStringC[3] = (char)(d_Wctr >> 8);
        ivStringC[4] = (char)d_Rctr;
        ivStringC[5] = (char)(d_Rctr >> 8);
        ivStringC[6] = (char)d_Wctr;
        ivStringC[7] = (char)(d_Wctr >> 8);
        ivStringC[8] = (char)d_Rctr;
        ivStringC[9] = (char)(d_Rctr >> 8);
        ivStringC[10] = (char)d_Wctr;
        ivStringC[11] = (char)(d_Wctr >> 8);

        return ivStringC;
    }
}