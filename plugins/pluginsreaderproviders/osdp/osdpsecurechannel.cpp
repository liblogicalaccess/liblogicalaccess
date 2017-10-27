/**
 * \file osdpreaderprovider.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief Reader Provider OSDP.
 */

#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "osdpsecurechannel.hpp"
#include "osdpreaderunitconfiguration.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include <openssl/rand.h>


namespace logicalaccess
{
void OSDPSecureChannel::deriveKey(std::shared_ptr<AES128Key> scbkkey,
                                  std::shared_ptr<AES128Key> scbkdkey)
{
    ByteVector d_lastIV(16);
    ByteVector inputData(16);

    unsigned char *keydata;
    if (isSCBK_D)
        keydata = scbkdkey->getData();
    else
        keydata = scbkkey->getData();

    openssl::SymmetricKey aeskey =
        static_cast<openssl::SymmetricKey>(openssl::AESSymmetricKey(
            openssl::AESSymmetricKey::createFromData(ByteVector(keydata, keydata + 16))));
    openssl::AESCipher cipher =
        openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB);
    openssl::InitializationVector iv = openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(d_lastIV));

    // SMAC1
    inputData[0] = 0x01;
    inputData[1] = 0x01;
    copy(m_CPChallenge.begin(), m_CPChallenge.begin() + 6, inputData.begin() + 2);
    cipher.cipher(inputData, m_smac1, aeskey, iv, false);

    inputData[0] = 0x01;
    inputData[1] = 0x02;
    copy(m_CPChallenge.begin(), m_CPChallenge.begin() + 6, inputData.begin() + 2);
    cipher.cipher(inputData, m_smac2, aeskey, iv, false);

    inputData[0] = 0x01;
    inputData[1] = 0x82;
    copy(m_CPChallenge.begin(), m_CPChallenge.begin() + 6, inputData.begin() + 2);
    cipher.cipher(inputData, m_senc, aeskey, iv, false);
}

void OSDPSecureChannel::computeAuthenticationData()
{
    ByteVector d_lastIV(16);
    ByteVector cryptogramInput, encCryptogramInput;

    cryptogramInput.insert(cryptogramInput.begin(), m_CPChallenge.begin(),
                           m_CPChallenge.end());
    cryptogramInput.insert(cryptogramInput.end(), m_PDChallenge.begin(),
                           m_PDChallenge.end());

    openssl::SymmetricKey aeskey = static_cast<openssl::SymmetricKey>(
        openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(m_senc)));
    openssl::AESCipher cipher =
        openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB);
    openssl::InitializationVector iv = openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(d_lastIV));

    cipher.cipher(cryptogramInput, encCryptogramInput, aeskey, iv, false);
    // CheckPDAuthentication
    if (!equal(encCryptogramInput.begin(), encCryptogramInput.end(),
               m_PDCryptogram.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cryptogram are not the same");

    cryptogramInput.clear();
    cryptogramInput.insert(cryptogramInput.begin(), m_PDChallenge.begin(),
                           m_PDChallenge.end());
    cryptogramInput.insert(cryptogramInput.end(), m_CPChallenge.begin(),
                           m_CPChallenge.end());
    // ComputeCPAuthenticationData
    cipher.cipher(cryptogramInput, m_CPCryptogram, aeskey, iv, false);
}

ByteVector OSDPSecureChannel::computeMAC(ByteVector data,
                                         openssl::AESInitializationVector iv) const
{
    openssl::AESCipher cipher =
        openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);

    if (data.size() % 16 != 0 || data.size() == 0x00)
    {
        data.push_back(0x80);
        while (data.size() % 16 != 0)
            data.push_back(0x00);
    }

    if (data.size() > 16)
    {
        openssl::SymmetricKey aeskey = static_cast<openssl::SymmetricKey>(
            openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(m_smac1)));
        // unsigned int length = data.size() - 16;
        ByteVector tmp(data.begin(), data.end() - 16), resEnc;
        cipher.cipher(tmp, resEnc, aeskey, iv, false);
        ByteVector newIv(resEnc.end() - 16, resEnc.end());
        iv = openssl::AESInitializationVector(
            openssl::AESInitializationVector::createFromData(newIv));
    }

    openssl::SymmetricKey aeskey = static_cast<openssl::SymmetricKey>(
        openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(m_smac2)));

    ByteVector lastData(data.end() - 16, data.end()), encData;
    cipher.cipher(lastData, encData, aeskey, iv, false);
    return encData;
}

ByteVector OSDPSecureChannel::computePacketMAC(ByteVector data)
{
    ByteVector mac =
        computeMAC(data, openssl::AESInitializationVector::createFromData(m_rmac));
    m_cmac = mac;
    return ByteVector(mac.begin(), mac.begin() + 4);
}

void OSDPSecureChannel::verifyMAC(ByteVector data)
{
    ByteVector expectedMac = ByteVector(data.end() - 4, data.end());
    ByteVector mac         = computeMAC(ByteVector(data.begin(), data.end() - 4),
                                openssl::AESInitializationVector::createFromData(m_cmac));

    if (!equal(expectedMac.begin(), expectedMac.end(), mac.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC are not the same");

    m_rmac = mac;
}

ByteVector OSDPSecureChannel::encryptData(ByteVector data, ByteVector ivArray) const
{
    ByteVector encData;
    openssl::AESCipher cipher =
        openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);
    openssl::SymmetricKey aeskey = static_cast<openssl::SymmetricKey>(
        openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(m_senc)));

    data.push_back(0x80);
    if (data.size() % 16 != 0 || data.size() == 0x00)
    {
        while (data.size() % 16 != 0)
            data.push_back(0x00);
    }

    for (unsigned char i = 0; i < 16; ++i)
        ivArray[i]       = ~ivArray[i];

    openssl::AESInitializationVector iv = openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(ivArray));

    cipher.cipher(data, encData, aeskey, iv, false);
    return encData;
}

ByteVector OSDPSecureChannel::decryptData(ByteVector data, ByteVector ivArray) const
{
    ByteVector decData;
    openssl::AESCipher cipher =
        openssl::AESCipher(openssl::OpenSSLSymmetricCipher::ENC_MODE_CBC);
    openssl::SymmetricKey aeskey = static_cast<openssl::SymmetricKey>(
        openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(m_senc)));

    for (unsigned char i = 0; i < 16; ++i)
        ivArray[i]       = ~ivArray[i];

    openssl::AESInitializationVector iv = openssl::AESInitializationVector(
        openssl::AESInitializationVector::createFromData(ivArray));
    cipher.decipher(data, decData, aeskey, iv, false);

    int i = (int)decData.size() - 1;
    for (; i >= 0 && decData[i] != 0x80 && decData[i] == 0x00; --i)
    {
    }
    if (i >= 0)
        decData.resize(i);

    return decData;
}
}