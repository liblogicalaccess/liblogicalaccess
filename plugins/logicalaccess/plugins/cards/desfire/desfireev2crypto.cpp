#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>
#include <algorithm>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <cassert>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>

namespace logicalaccess
{
ByteVector DESFireEV2Crypto::get_challenge_response(ByteVector rndb,
                                                    const ByteVector &rnda)
{
    std::rotate(rndb.begin(), rndb.begin() + 1, rndb.end());

    auto full = rnda;
    full.insert(full.end(), rndb.begin(), rndb.end());
    return full;
}

ByteVector DESFireEV2Crypto::auth_ev2_part2(const ByteVector &data,
                                            const ByteVector &rnda)
{
    auto TI    = ByteVector(data.begin(), data.begin() + 4);
    auto rnda2 = ByteVector(data.begin() + 4, data.begin() + 20);

    // We ignore the 6 + 6 bytes of PCD/PICC capabilities.
    std::rotate(rnda2.rbegin(), rnda2.rbegin() + 1, rnda2.rend());
    EXCEPTION_ASSERT_WITH_LOG(rnda == rnda2, LibLogicalAccessException,
                              "RNDA and RNDA' don't match. The card may be invalid.");

    return TI;
}

std::tuple<ByteVector, ByteVector> DESFireEV2Crypto::getSV(const ByteVector &rnda,
                                                           const ByteVector &rndb)
{
    ByteVector SV1, emptyIV(16), kdf_SV1_input = {0xA5, 0x5A, 0x00, 0x01, 0x00, 0x80},
                                 kdf_SV2_input = {0x5A, 0xA5, 0x00, 0x01, 0x00, 0x80};


    // SV1
    SV1.insert(SV1.end(), rnda.begin(), rnda.begin() + 2);
    SV1.insert(SV1.end(), rnda.begin() + 2, rnda.begin() + 8);

    for (unsigned char x = 0; x <= 5; ++x)
    {
        SV1[x + 2] ^= rndb[x];
    }

    SV1.insert(SV1.end(), rndb.begin() + 6, rndb.end());
    SV1.insert(SV1.end(), rnda.begin() + 8, rnda.end());

    // SV2
    ByteVector SV2 = SV1;

    SV1.insert(SV1.begin(), kdf_SV1_input.begin(), kdf_SV1_input.end());
    SV2.insert(SV2.begin(), kdf_SV2_input.begin(), kdf_SV2_input.end());

    return std::tuple<ByteVector, ByteVector>(SV1, SV2);
}

void DESFireEV2Crypto::generateSessionKey(const ByteVector &key_data,
                                          const ByteVector &rnda, const ByteVector &rndb)
{
    auto sv = getSV(rnda, rndb);

    d_sessionKey    = openssl::CMACCrypto::cmac(key_data, "aes", std::get<0>(sv), {});
    d_macSessionKey = openssl::CMACCrypto::cmac(key_data, "aes", std::get<1>(sv), {});

    cmdctr_ = 0;
    d_cipher.reset(new openssl::AESCipher());
    d_mac_size   = 8;
}

void DESFireEV2Crypto::sam_generateSessionKey(const ByteVector &rnda,
                                              const ByteVector &rndb,
                                              std::shared_ptr<SAMAV2ISO7816Commands> cmd)
{
    auto sv = getSV(rnda, rndb);

    d_sessionKey    = cmd->cmacOffline(std::get<0>(sv));
    d_macSessionKey = cmd->cmacOffline(std::get<1>(sv));

    cmdctr_ = 0;
    d_cipher.reset(new openssl::AESCipher());
    d_mac_size   = 8;
}

ByteVector DESFireEV2Crypto::truncateMAC(const ByteVector &full_mac)
{
    assert(full_mac.size() % 2 == 0);
    ByteVector truncated(full_mac.size() / 2);
    unsigned char truncateCount = 0;
    unsigned char count         = 1;

    while (count < full_mac.size())
    {
        truncated[truncateCount] = full_mac[count];
        count += 2;
        ++truncateCount;
    }
    return truncated;
}

ByteVector DESFireEV2Crypto::getIVEncrypt(bool cmdData)
{
    // Calcul IV for encrypt
    const auto isokey = std::make_shared<openssl::AESSymmetricKey>(
        openssl::AESSymmetricKey::createFromData(d_sessionKey));
    const auto iv = std::make_shared<openssl::AESInitializationVector>(
        openssl::AESInitializationVector::createNull());
    auto cipher = std::make_shared<openssl::AESCipher>(
        openssl::OpenSSLSymmetricCipher::ENC_MODE_ECB);

    ByteVector cmdCtrVector, IVdata, IV;
    const ByteVector labelRespData = {0x5A, 0xA5};
    const ByteVector labelCmdData  = {0xA5, 0x5A};

    if (cmdData)
        IVdata = labelCmdData;
    else
        IVdata = labelRespData;

    IVdata.insert(IVdata.end(), ti_.begin(), ti_.end()); // TI
    BufferHelper::setUInt16(cmdCtrVector, cmdctr_);
    IVdata.insert(IVdata.end(), cmdCtrVector.begin(), cmdCtrVector.end()); // CMDCtr
    IVdata.resize(cipher->getBlockSize());

    cipher->cipher(IVdata, IV, *isokey, *iv, false);

    return IV;
}

ByteVector DESFireEV2Crypto::generateMAC(unsigned char cmd, const ByteVector &buf)
{
    if (d_auth_method != CryptoMethod::CM_EV2)
        return DESFireCrypto::generateMAC(cmd, buf);

    ByteVector command, cmdCtrVector;
    auto do_mac = ByteVector{};
    do_mac.push_back(cmd);
    BufferHelper::setUInt16(cmdCtrVector, cmdctr_);
    do_mac.insert(do_mac.end(), cmdCtrVector.begin(), cmdCtrVector.end()); // CMDCtr
    do_mac.insert(do_mac.end(), ti_.begin(), ti_.end());                   // TI
    do_mac.insert(do_mac.end(), buf.begin(), buf.end());                   // CMD Data

    auto m = openssl::CMACCrypto::cmac(d_macSessionKey, "aes", do_mac, {});
    m = truncateMAC(m);

    command.insert(command.end(), m.begin(), m.begin() + d_mac_size);

    return command;
}

bool DESFireEV2Crypto::verifyMAC(bool end, const ByteVector &data)
{
    if (d_auth_method != CryptoMethod::CM_EV2)
        return DESFireCrypto::verifyMAC(end, data);

    if (data.size() < d_mac_size)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "verifyMAC buffer param is too small.");

    ByteVector mac            = ByteVector(data.end() - d_mac_size, data.end());
    const ByteVector respData = ByteVector(data.begin(), data.end() - d_mac_size);

    ++cmdctr_;

    auto m = generateMAC(0x00, respData);

    if (!std::equal(m.begin(), m.begin() + d_mac_size, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "verifyMAC command MAC answer has an unexpected value.");

    return true;
}

ByteVector DESFireEV2Crypto::desfireEncrypt(const ByteVector &data,
                                            const ByteVector &param, bool calccrc)
{
    if (d_auth_method != CryptoMethod::CM_EV2)
        return DESFireCrypto::desfireEncrypt(data, param, calccrc);

    const auto IV = getIVEncrypt(true);

    // Encrypt
    const auto isokey = std::make_shared<openssl::AESSymmetricKey>(
        openssl::AESSymmetricKey::createFromData(d_sessionKey));
    const auto iv = std::make_shared<openssl::AESInitializationVector>(
        openssl::AESInitializationVector::createFromData(IV));

    ByteVector encdata;
    ByteVector dataCalc = data;

    if (dataCalc.size() % d_cipher->getBlockSize() != 0)
    {
        dataCalc.push_back(0x80);
        if (dataCalc.size() % d_cipher->getBlockSize() != 0)
            dataCalc.resize(
                static_cast<unsigned char>(data.size() / d_cipher->getBlockSize() + 1) *
                d_cipher->getBlockSize());
    }

    d_cipher->cipher(dataCalc, encdata, *isokey, *iv, false);

    ByteVector result = encdata;
    if (param.size() > 1)
        encdata.insert(encdata.begin(), param.begin() + 1, param.end());
    auto m = generateMAC(param[0], encdata);
    result.insert(result.end(), m.begin(), m.end());

    return result;
}

ByteVector DESFireEV2Crypto::desfireDecrypt(size_t length)
{
    if (d_auth_method != CryptoMethod::CM_EV2)
        return DESFireCrypto::desfireDecrypt(length);

    verifyMAC(true, d_buf);

    const auto IV = getIVEncrypt(false);

    // Decipher
    const auto isokey = std::make_shared<openssl::AESSymmetricKey>(
        openssl::AESSymmetricKey::createFromData(d_sessionKey));
    const auto iv = std::make_shared<openssl::AESInitializationVector>(
        openssl::AESInitializationVector::createFromData(IV));

    const auto encData = ByteVector(d_buf.begin(), d_buf.end() - d_mac_size);
    ByteVector data, macCheck;

    d_cipher->decipher(encData, data, *isokey, *iv, false);

    size_t ll = data.size() - 1;

    while (data[ll] == 0x00)
    {
        --ll;
    }

    EXCEPTION_ASSERT_WITH_LOG(data[ll] == 0x80, LibLogicalAccessException,
                              "Incorrect FLT result");

    data.resize(ll);

    return data;
}

ByteVector DESFireEV2Crypto::changeKey_PICC(unsigned char keyno,
                                            ByteVector oldKeyDiversify,
                                            std::shared_ptr<DESFireKey> newkey,
                                            ByteVector newKeyDiversify,
                                            unsigned char keysetno)
{
    const unsigned char keyno_only = static_cast<unsigned char>(keyno & 0x3F);

    if (d_auth_method != CryptoMethod::CM_EV2 ||
        (keyno_only == d_currentKeyNo && !keysetno)) // ChangeKeyEV2
        return DESFireCrypto::changeKey_PICC(keyno, oldKeyDiversify, newkey,
                                             newKeyDiversify, keysetno);

    // We only handle when keys number are different here
    LOG(LogLevel::INFOS) << "Init change key on PICC...";
    ByteVector oldkeydiv, newkeydiv;
    oldkeydiv.resize(16, 0x00);
    newkeydiv.resize(16, 0x00);
    // Get keyno only, in case of master card key
    getKey(keysetno, keyno, oldKeyDiversify, oldkeydiv);
    getKey(newkey, newKeyDiversify, newkeydiv);

    ByteVector encCryptogram;
    for (unsigned int i = 0; i < newkeydiv.size(); ++i)
    {
        encCryptogram.push_back(static_cast<unsigned char>(oldkeydiv[i] ^ newkeydiv[i]));
    }

    if (newkey->getKeyType() == DF_KEY_AES)
    {
        // For AES, add key version.
        encCryptogram.push_back(newkey->getKeyVersion());
    }

    const uint32_t crc = desfire_crc32(&newkeydiv[0], newkeydiv.size());
    encCryptogram.push_back(static_cast<unsigned char>(crc & 0xff));
    encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
    encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff0000) >> 16));
    encCryptogram.push_back(static_cast<unsigned char>((crc & 0xff000000) >> 24));

    auto param = ByteVector({DF_INS_CHANGE_KEY, keyno});
    if (keysetno != 0) // ChangeKeyEV2
    {
        param[0] = DFEV2_INS_CHANGEKEY_EV2;
        param.insert(param.begin() + 1, keysetno);
    }
    ByteVector cryptogram = desfireEncrypt(encCryptogram, param);

    return cryptogram;
}

void DESFireEV2Crypto::duplicateCurrentKeySet(uint8_t keySetNb)
{
    duplicateKeySet(0, keySetNb);
}

void DESFireEV2Crypto::setKeySetCryptoType(uint8_t keySetNb, DESFireKeyType cryptoMethod)
{
    for (auto &&key : d_keys)
    {
        if (std::get<0>(key.first) == d_currentAid && std::get<1>(key.first) == keySetNb)
            key.second->setKeyType(cryptoMethod);
    }
}

void DESFireEV2Crypto::duplicateKeySet(uint8_t keySetNbToDuplicate, uint8_t keySetNb)
{
    uint8_t nbKeys = 0;
    for (auto &&key : d_keys)
    {
        if (std::get<0>(key.first) == d_currentAid &&
            std::get<1>(key.first) == keySetNbToDuplicate)
        {
            const uint8_t tmp = std::get<2>(key.first);
            if (nbKeys < tmp)
                nbKeys = tmp;
        }
    }

    for (auto x = 0; x <= nbKeys; ++x)
        d_keys[std::make_tuple(d_currentAid, keySetNb, x)] = std::make_shared<DESFireKey>(
            *d_keys[std::make_tuple(d_currentAid, keySetNbToDuplicate, x)]);
}
} // namespace logicalaccess
