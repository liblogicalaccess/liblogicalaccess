#include <logicalaccess/plugins/cards/desfire/desfireev2crypto.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <logicalaccess/plugins/crypto/secure_memory.hpp>

namespace logicalaccess
{
ByteVector DESFireEV2Crypto::get_challenge_response(ByteVector rndb,
                                                    const ByteVector &rnda)
{
    EXCEPTION_ASSERT_WITH_LOG(rnda.size() == 16, LibLogicalAccessException,
                              "EV2 RndA must contain exactly 16 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(rndb.size() == 16, LibLogicalAccessException,
                              "EV2 RndB must contain exactly 16 bytes.");
    security::SecureZeroGuard rndbGuard{&rndb};
    std::rotate(rndb.begin(), rndb.begin() + 1, rndb.end());

    ByteVector full;
    full.reserve(32);
    full.insert(full.end(), rnda.begin(), rnda.end());
    full.insert(full.end(), rndb.begin(), rndb.end());
    return full;
}

ByteVector DESFireEV2Crypto::rotateAuthenticationChallenge(const ByteVector &rnd)
{
    EXCEPTION_ASSERT_WITH_LOG(rnd.size() == 16, LibLogicalAccessException,
                              "EV2 authentication challenge must contain exactly 16 bytes.");
    ByteVector rotated = rnd;
    std::rotate(rotated.begin(), rotated.begin() + 1, rotated.end());
    return rotated;
}

DESFireEV2Crypto::AuthenticationResponse DESFireEV2Crypto::parseEV2FirstResponse(
    const ByteVector &data, const ByteVector &rnda,
    const ByteVector &expectedPCDCap2, bool validateCapabilities)
{
    EXCEPTION_ASSERT_WITH_LOG(data.size() == 32, LibLogicalAccessException,
                              "EV2 First response must contain exactly 32 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(expectedPCDCap2.size() <= 6, LibLogicalAccessException,
                              "EV2 PCDcap2 must contain at most six bytes.");

    ByteVector expectedRndA = rotateAuthenticationChallenge(rnda);
    security::SecureZeroGuard rndAGuard{&expectedRndA};
    EXCEPTION_ASSERT_WITH_LOG(std::equal(expectedRndA.begin(), expectedRndA.end(), data.begin() + 4),
                              LibLogicalAccessException, "EV2 authentication failed: invalid rotated RndA.");

    AuthenticationResponse result{
        ByteVector(data.begin(), data.begin() + 4),
        ByteVector(data.begin() + 20, data.begin() + 26),
        ByteVector(data.begin() + 26, data.end())};
    if (validateCapabilities)
    {
        ByteVector adjustedCapabilities = expectedPCDCap2;
        adjustedCapabilities.resize(6, 0);
        EXCEPTION_ASSERT_WITH_LOG(result.pcdCap2 == adjustedCapabilities,
                                  LibLogicalAccessException, "EV2 returned PCDcap2 does not match the sent capabilities.");
    }
    return result;
}

ByteVector DESFireEV2Crypto::auth_ev2_part2(const ByteVector &data,
                                            const ByteVector &rnda)
{
    return parseEV2FirstResponse(data, rnda).ti;
}

ByteVector DESFireEV2Crypto::buildKDFContext(const ByteVector &rnda, const ByteVector &rndb)
{
    EXCEPTION_ASSERT_WITH_LOG(rnda.size() == 16 && rndb.size() == 16, LibLogicalAccessException,
                              "EV2 KDF requires two 16-byte authentication challenges.");
    ByteVector context;
    context.reserve(26);
    context.insert(context.end(), rnda.begin(), rnda.begin() + 2);
    for (std::size_t i = 0; i < 6; ++i)
        context.push_back(static_cast<std::uint8_t>(rnda[2 + i] ^ rndb[i]));
    context.insert(context.end(), rndb.begin() + 6, rndb.end());
    context.insert(context.end(), rnda.begin() + 8, rnda.end());
    return context;
}

ByteVector DESFireEV2Crypto::buildKDFVector(std::uint8_t label0, std::uint8_t label1,
                                          std::uint16_t counter, std::uint16_t length,
                                          const ByteVector &context)
{
    ByteVector vector = {label0, label1,
        static_cast<std::uint8_t>(counter >> 8U), static_cast<std::uint8_t>(counter & 0xFFU),
        static_cast<std::uint8_t>(length >> 8U), static_cast<std::uint8_t>(length & 0xFFU)};
    vector.reserve(32);
    vector.insert(vector.end(), context.begin(), context.end());
    return vector;
}

std::tuple<ByteVector, ByteVector> DESFireEV2Crypto::getSV(const ByteVector &rnda,
                                                        const ByteVector &rndb)
{
    ByteVector context = buildKDFContext(rnda, rndb);
    security::SecureZeroGuard contextGuard{&context};
    return std::make_tuple(buildKDFVector(0xA5, 0x5A, 1, 128, context),
                           buildKDFVector(0x5A, 0xA5, 1, 128, context));
}

std::pair<ByteVector, ByteVector> DESFireEV2Crypto::deriveEV2SessionKeys(
    const ByteVector &key, const ByteVector &rnda, const ByteVector &rndb)
{
    EXCEPTION_ASSERT_WITH_LOG(key.size() == 16 || key.size() == 32, LibLogicalAccessException,
                              "EV2 KDF requires an AES-128 or AES-256 key.");
    ByteVector context = buildKDFContext(rnda, rndb);
    security::SecureZeroGuard contextGuard{&context};
    const auto length = static_cast<std::uint16_t>(key.size() * 8);
    std::pair<ByteVector, ByteVector> keys;
    try
    {
        keys.first.reserve(key.size());
        keys.second.reserve(key.size());
        for (std::uint16_t counter = 1; counter <= key.size() / 16; ++counter)
        {
            ByteVector encVector = buildKDFVector(0xA5, 0x5A, counter, length, context);
            security::SecureZeroGuard encVectorGuard{&encVector};
            ByteVector macVector = buildKDFVector(0x5A, 0xA5, counter, length, context);
            security::SecureZeroGuard macVectorGuard{&macVector};
            ByteVector enc = openssl::CMACCrypto::cmac(key, "aes", encVector, {});
            security::SecureZeroGuard encGuard{&enc};
            ByteVector mac = openssl::CMACCrypto::cmac(key, "aes", macVector, {});
            security::SecureZeroGuard macGuard{&mac};
            EXCEPTION_ASSERT_WITH_LOG(enc.size() == 16 && mac.size() == 16, LibLogicalAccessException,
                                      "EV2 KDF returned an invalid CMAC block length.");
            keys.first.insert(keys.first.end(), enc.begin(), enc.end());
            keys.second.insert(keys.second.end(), mac.begin(), mac.end());
        }
        return keys;
    }
    catch (...)
    {
        security::secureZeroBuffer(keys.first);
        security::secureZeroBuffer(keys.second);
        throw;
    }
}

void DESFireEV2Crypto::generateSessionKey(const ByteVector &key_data,
                                          const ByteVector &rnda, const ByteVector &rndb)
{
    EXCEPTION_ASSERT_WITH_LOG(key_data.size() == 16, LibLogicalAccessException,
                              "DESFire EV2 session-key generation requires an AES-128 key.");
    auto keys = deriveEV2SessionKeys(key_data, rnda, rndb);
    security::SecureZeroGuard keysGuard{&keys.first, &keys.second};
    replaceSessionKeys(keys);
    setCmdCtr(0);
}

void DESFireEV2Crypto::replaceSessionKeys(const std::pair<ByteVector, ByteVector> &keys)
{
    EXCEPTION_ASSERT_WITH_LOG((keys.first.size() == 16 || keys.first.size() == 32) &&
                              keys.first.size() == keys.second.size(), LibLogicalAccessException,
                              "EV2 session keys must have matching AES-128 or AES-256 lengths.");
    ByteVector enc = keys.first;
    security::SecureZeroGuard encGuard{&enc};
    ByteVector mac = keys.second;
    security::SecureZeroGuard macGuard{&mac};
    auto cipher = std::make_shared<openssl::AESCipher>();
    d_sessionKey.swap(enc);
    d_macSessionKey.swap(mac);
    d_cipher = std::move(cipher);
    d_mac_size = 8;
}

void DESFireEV2Crypto::installFirstSession(const std::pair<ByteVector, ByteVector> &keys,
                                          const ByteVector &ti, CryptoMethod method, std::uint8_t keyno)
{
    EXCEPTION_ASSERT_WITH_LOG(ti.size() == 4 && usesEV2Crypto(method), LibLogicalAccessException,
                              "EV2 First session requires a four-byte TI and EV2-compatible authentication mode.");
    EXCEPTION_ASSERT_WITH_LOG(method != CM_EV2 || keys.first.size() == 16, LibLogicalAccessException,
                              "DESFire EV2 sessions require AES-128 session keys.");
    try
    {
        invalidateAuthentication();
        replaceSessionKeys(keys);
        setTI(ti);
        d_auth_method = method;
        d_currentKeyNo = keyno;
    }
    catch (...)
    {
        invalidateAuthentication();
        throw;
    }
}

void DESFireEV2Crypto::installNonFirstSession(const std::pair<ByteVector, ByteVector> &keys,
                                             std::uint8_t keyno)
{
    EXCEPTION_ASSERT_WITH_LOG(usesEV2Crypto(d_auth_method) && ti_.size() == 4, LibLogicalAccessException,
                              "EV2 NonFirst requires an active EV2-compatible session.");
    replaceSessionKeys(keys);
    security::secureZeroBuffer(d_lastIV);
    d_lastIV.clear();
    security::secureZeroBuffer(d_last_left);
    d_last_left.clear();
    security::secureZeroBuffer(d_buf);
    d_buf.clear();
    d_currentKeyNo = keyno;
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
    EXCEPTION_ASSERT_WITH_LOG(full_mac.size() % 2U == 0U,
        LibLogicalAccessException, "DESFire EV2 : Unexpected AES-CMAC size.");

    ByteVector truncated(full_mac.size() / 2U);

    for (std::size_t i = 0U; i < truncated.size(); ++i)
        truncated[i] = full_mac[(2U * i) + 1U];

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

    ByteVector cmdCtrVector, IV;
    //CmdData or RespData
    const ByteVector label = cmdData ? ByteVector{0xA5, 0x5A} : ByteVector{0x5A, 0xA5};

    ByteVector IVdata = label;

    IVdata.insert(IVdata.end(), ti_.begin(), ti_.end()); // TI
    BufferHelper::setUInt16(cmdCtrVector, cmdctr_);
    IVdata.insert(IVdata.end(), cmdCtrVector.begin(), cmdCtrVector.end()); // CMDCtr
    IVdata.resize(cipher->getBlockSize());

    cipher->cipher(IVdata, IV, *isokey, *iv, false);

    return IV;
}

ByteVector DESFireEV2Crypto::generateMAC(unsigned char cmd, const ByteVector &buf)
{
    if (!usesEV2Crypto(d_auth_method))
        return DESFireCrypto::generateMAC(cmd, buf);

    ByteVector command, cmdCtrVector;
    auto do_mac = ByteVector{};
    do_mac.push_back(cmd);
    BufferHelper::setUInt16(cmdCtrVector, cmdctr_);
    do_mac.insert(do_mac.end(), cmdCtrVector.begin(), cmdCtrVector.end()); // CMDCtr
    do_mac.insert(do_mac.end(), ti_.begin(), ti_.end());                   // TI
    do_mac.insert(do_mac.end(), buf.begin(), buf.end());                   // CMD Data

    auto mac = openssl::CMACCrypto::cmac(d_macSessionKey, "aes", do_mac, {});
    mac = truncateMAC(mac);

    command.insert(command.end(), mac.begin(), mac.begin() + d_mac_size);

    return command;
}

bool DESFireEV2Crypto::verifyMAC(bool end, const ByteVector &data)
{
    if (!usesEV2Crypto(d_auth_method))
        return DESFireCrypto::verifyMAC(end, data);

    if (!data.empty())
        d_buf.insert(d_buf.end(), data.begin(), data.end());

    if (!end)
        return true;

    LOG(LogLevel::INFOS) << "Running VerifyMAC on buffer: " << d_buf;

    if (d_buf.size() < d_mac_size)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "verifyMAC buffer param is too small.");

    const ByteVector mac(d_buf.end() - d_mac_size, d_buf.end());
    const ByteVector respData(d_buf.begin(), d_buf.end() - d_mac_size);
    d_buf.clear();

    ++cmdctr_;

    const auto expectedMac = generateMAC(0x00, respData);

    if (!std::equal(expectedMac.begin(), expectedMac.begin() + d_mac_size, mac.begin()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "verifyMAC command MAC answer has an unexpected value.");

    return true;
}

ByteVector DESFireEV2Crypto::desfireEncrypt(const ByteVector &data, const ByteVector &param, bool calccrc)
{
    if (!usesEV2Crypto(d_auth_method))
        return DESFireCrypto::desfireEncrypt(data, param, calccrc);

    ByteVector encdata;

    // An empty command payload remains empty
    // No padding and no AES encryption are performed
    if (!data.empty())
    {
        const auto IV = getIVEncrypt(true);

        const auto isokey = std::make_shared<openssl::AESSymmetricKey>(
            openssl::AESSymmetricKey::createFromData(d_sessionKey));
        const auto iv = std::make_shared<openssl::AESInitializationVector>(
            openssl::AESInitializationVector::createFromData(IV));

        ByteVector dataCalc = data;

        const std::size_t blockSize = d_cipher->getBlockSize();

        dataCalc.push_back(0x80);

        const std::size_t remainder = dataCalc.size() % blockSize;

        if (remainder != 0)
            dataCalc.resize(dataCalc.size() + (blockSize - remainder), 0x00);

        d_cipher->cipher(dataCalc, encdata, *isokey, *iv, false);
    }

    ByteVector macData = encdata;

    if (param.size() > 1)
        macData.insert(macData.begin(), param.begin() + 1, param.end());
    const auto mac = generateMAC(param[0], macData);
    ByteVector result = encdata;
    result.insert(result.end(), mac.begin(), mac.end());

    return result;
}

ByteVector DESFireEV2Crypto::desfireDecrypt(std::size_t length)
{
    if (!usesEV2Crypto(d_auth_method))
        return DESFireCrypto::desfireDecrypt(length);

    const ByteVector response = d_buf;

    EXCEPTION_ASSERT_WITH_LOG(response.size() >= d_mac_size,
        LibLogicalAccessException, "DESFire EV2 : Response is shorter than the expected MAC.");

    verifyMAC(true, ByteVector{}); // Verify the response MAC before decrypting its payload

    const std::size_t encryptedLength = response.size() - d_mac_size;

    if (encryptedLength == 0U) // MAC-only response
        return ByteVector{};

    const ByteVector encryptedData(response.begin(), response.begin() + encryptedLength);
    const auto IV = getIVEncrypt(false);
    // Decipher
    const auto isokey = std::make_shared<openssl::AESSymmetricKey>(
        openssl::AESSymmetricKey::createFromData(d_sessionKey));
    const auto iv = std::make_shared<openssl::AESInitializationVector>(
        openssl::AESInitializationVector::createFromData(IV));
    
    ByteVector data;
    d_cipher->decipher(encryptedData, data, *isokey, *iv, false);

    if (length != 0U)
    {
        EXCEPTION_ASSERT_WITH_LOG(data.size() >= length,
            LibLogicalAccessException, "DESFire EV2 : Decrypted buffer is smaller than the expected length.");

        data.resize(length);
        return data;
    }

    // Remove ISO/IEC 7816-4 padding safely : trailing 0x00 bytes followed by a 0x80 marker.
    std::size_t unpaddedLength = data.size();

    while (unpaddedLength > 0U && data[unpaddedLength - 1U] == 0x00U)
        --unpaddedLength;

    EXCEPTION_ASSERT_WITH_LOG(unpaddedLength > 0U && data[unpaddedLength - 1U] == 0x80U,
        LibLogicalAccessException, "DESFire EV2 : Invalid ISO/IEC 7816-4 padding.");

    // Exclude the 0x80 marker and trailing zeros
    data.resize(unpaddedLength - 1U);

    return data;
}

ByteVector DESFireEV2Crypto::changeKey_PICC(unsigned char keyno,
                                            ByteVector oldKeyDiversify,
                                            std::shared_ptr<DESFireKey> newkey,
                                            ByteVector newKeyDiversify,
                                            unsigned char keysetno)
{
    const unsigned char keyno_only = static_cast<unsigned char>(keyno & 0x3F);
    if (!usesEV2Crypto(d_auth_method) || (keyno_only == d_currentKeyNo && !keysetno)) // ChangeKeyEV2
        return DESFireCrypto::changeKey_PICC(keyno, oldKeyDiversify, newkey, newKeyDiversify, keysetno);

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

void DESFireEV2Crypto::duplicateCurrentKeySet(std::uint8_t keySetNb)
{
    duplicateKeySet(0, keySetNb);
}

void DESFireEV2Crypto::setKeySetCryptoType(std::uint8_t keySetNb, DESFireKeyType cryptoMethod)
{
    for (auto &&key : d_keys)
    {
        if (std::get<0>(key.first) == d_currentAid && std::get<1>(key.first) == keySetNb)
            key.second->setKeyType(cryptoMethod);
    }
}

void DESFireEV2Crypto::duplicateKeySet(std::uint8_t keySetNbToDuplicate, std::uint8_t keySetNb)
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
    {
        auto oIndex = std::make_tuple(d_currentAid, keySetNbToDuplicate, x);
        if (d_keys.find(oIndex) != d_keys.end())
            d_keys[std::make_tuple(d_currentAid, keySetNb, x)] = std::make_shared<DESFireKey>(*d_keys[oIndex]);
    }
}

void DESFireEV2Crypto::invalidateAuthentication()
{
    // Wipe authentication and session-sensitive material before releasing it
    auto secureClear = [](ByteVector &v)
    {
        logicalaccess::security::secureZeroBuffer(v);
        v.clear();
    };

    // Session keys and authentication key
    secureClear(d_sessionKey);
    secureClear(d_macSessionKey);
    secureClear(d_authkey);

    // Authentication nonces
    secureClear(d_rndA);
    secureClear(d_rndB);

    // EV2 transaction state
    secureClear(ti_);
    cmdctr_ = 0;

    // Secure-messaging and chaining state
    secureClear(d_lastIV);
    secureClear(d_last_left);

    // Pending protocol data
    secureClear(d_buf);

    // No authenticated cipher remains.
    d_cipher.reset();

    // Return to the unauthenticated/default protocol state
    d_auth_method  = CM_LEGACY;
    d_currentKeyNo = 0;
    d_mac_size     = 4;

    // Keep d_currentAid and d_keys intact because it's selection state, not authentication state
}

void DESFireEV2Crypto::initializeSecureMessagingState()
{
    cmdctr_ = 0;
    d_cipher.reset(new openssl::AESCipher());
    d_mac_size = 8;
}

} // namespace logicalaccess
