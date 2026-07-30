/**
 * \file SAMAV3ISO7816Commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3ISO7816Commands commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/samav3iso7816commands.hpp>

namespace logicalaccess
{
SAMAV3ISO7816Commands::SAMAV3ISO7816Commands()
    : SAMAV2ISO7816Commands(CMD_SAMAV3ISO7816)
{
}

SAMAV3ISO7816Commands::SAMAV3ISO7816Commands(std::string ct)
    : SAMAV2ISO7816Commands(ct)
{
}

SAMAV3ISO7816Commands::~SAMAV3ISO7816Commands() {}

ByteVector SAMAV3ISO7816Commands::encipherKeyEntry(
    unsigned char keyno, unsigned char targetKeyno, unsigned short changeCounter,
    unsigned char channel, const ByteVector &targetSamUid, const ByteVector &divInput)
{
    unsigned char p2 = 0x00;
    ByteVector data;
    data.reserve(4 + targetSamUid.size() + divInput.size());
    data.push_back(static_cast<unsigned char>(0x80 | (channel & 0x03)));
    data.push_back(targetKeyno);
    sam::appendUInt16BE(data, changeCounter);
    if (!targetSamUid.empty())
    {
        p2 |= 0x01;
        data.insert(data.end(), targetSamUid.begin(), targetSamUid.end());
    }
    if (!divInput.empty())
    {
        p2 |= 0x02;
        data.insert(data.end(), divInput.begin(), divInput.end());
    }
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        d_cla, sam::ins::key::EncipherKeyEntry, keyno, p2, static_cast<unsigned char>(data.size()), data, 0x00);
    return result.getData();
}

void SAMAV3ISO7816Commands::PKI_ImportCaPk(const ByteVector &rid, unsigned char pkId, unsigned short set,
                                           unsigned char keyNoCEK, unsigned char keyVCEK,
                                           unsigned char keyNoAEK, unsigned char keyVAEK,
                                           unsigned char pkExpLen, const ByteVector &pk,
                                           const ByteVector &checkSum, bool settingsOnly)
{
    constexpr std::uint16_t SW_UNKNOWN_PK_INDEX = 0x6A83;
    constexpr std::size_t RID_SIZE              = 5;
    constexpr std::size_t CHECKSUM_SIZE         = 20;
    constexpr std::size_t BASE_PAYLOAD_SIZE     = 12;
    constexpr std::size_t FIXED_CRYPTO_SIZE     = 24;
    constexpr std::size_t MIN_RSA_MODULUS_SIZE  = 0x40;
    constexpr std::size_t MAX_RSA_MODULUS_SIZE  = 0xF8;
    constexpr unsigned char EXPONENT_SHORT_SIZE = 1;
    constexpr unsigned char EXPONENT_LONG_SIZE  = 3;
    constexpr unsigned char ALGO_HASH           = 0x01;
    constexpr unsigned char ALGO_PK             = 0x01;

    EXCEPTION_ASSERT_WITH_LOG(rid.size() == RID_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "RID must be exactly 5 bytes."));

    EXCEPTION_ASSERT_WITH_LOG(!settingsOnly || (pk.empty() && checkSum.empty()), LibLogicalAccessException,
        sam::errorMessage(__func__, "Crypto parameters must not be provided when settingsOnly = true."));

    if (!settingsOnly)
    {
        EXCEPTION_ASSERT_WITH_LOG(!pk.empty() && pk.size() >= MIN_RSA_MODULUS_SIZE && pk.size() <= MAX_RSA_MODULUS_SIZE,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid CA public key modulus length."));

        EXCEPTION_ASSERT_WITH_LOG(pkExpLen == EXPONENT_SHORT_SIZE || pkExpLen == EXPONENT_LONG_SIZE,
            LibLogicalAccessException, sam::errorMessage(__func__, "Invalid public exponent length."));

        EXCEPTION_ASSERT_WITH_LOG(checkSum.size() == CHECKSUM_SIZE,
            LibLogicalAccessException, sam::errorMessage(__func__, "Checksum must be exactly 20 bytes."));
    }

    ByteVector payload;
    payload.reserve(settingsOnly ? BASE_PAYLOAD_SIZE : BASE_PAYLOAD_SIZE + FIXED_CRYPTO_SIZE + pk.size() + pkExpLen);
    payload.insert(payload.end(), rid.begin(), rid.end());
    payload.push_back(pkId);
    sam::appendUInt16BE(payload, set);
    payload.push_back(keyNoCEK);
    payload.push_back(keyVCEK);
    payload.push_back(keyNoAEK);
    payload.push_back(keyVAEK);
    if (!settingsOnly)
    {
        payload.push_back(ALGO_HASH);
        payload.push_back(ALGO_PK);
        payload.push_back(static_cast<unsigned char>(pk.size()));
        payload.insert(payload.end(), pk.begin(), pk.end());
        payload.push_back(pkExpLen);
        if (pkExpLen == EXPONENT_SHORT_SIZE)
            payload.push_back(0x03);
        else
            payload.insert(payload.end(), {0x01, 0x00, 0x01});
        payload.insert(payload.end(), checkSum.begin(), checkSum.end());
    }

    const bool settingsPayload = payload.size() == BASE_PAYLOAD_SIZE;
    const std::size_t MIN_PAYLOAD_SIZE = BASE_PAYLOAD_SIZE + 5 + MIN_RSA_MODULUS_SIZE + CHECKSUM_SIZE;
    const std::size_t MAX_PAYLOAD_SIZE = BASE_PAYLOAD_SIZE + 7 + MAX_RSA_MODULUS_SIZE + CHECKSUM_SIZE;
    const bool keyPayload = !settingsOnly && payload.size() >= MIN_PAYLOAD_SIZE && payload.size() <= MAX_PAYLOAD_SIZE;

    EXCEPTION_ASSERT_WITH_LOG(settingsPayload || keyPayload,
        LibLogicalAccessException, sam::errorMessage(__func__, "Payload size does not match PKI Import CA PK format."));

    const unsigned char lc =
        payload.size() > sam::MAX_SECURE_APDU_DATA_SIZE ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::ImportCaPk);
    apdu.push_back(0x00);
    apdu.push_back(settingsOnly ? 0x80 : 0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Extended, sam::EMV_LAYOUT);

    if (sam::parseStatusWord(response) == SW_UNKNOWN_PK_INDEX)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Unknown PK index."));

    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV3ISO7816Commands::PKI_ImportCaPkOffline(const ByteVector &offlineCryptogram,
                                             bool settingsOnly, bool expectResponseData)
{
    EXCEPTION_ASSERT_WITH_LOG(!offlineCryptogram.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Offline cryptogram cannot be empty."));

    constexpr std::size_t OFFLINE_CRYPTOGRAM_SHORT    = 26;
    constexpr std::size_t OFFLINE_CRYPTOGRAM_MEDIUM   = 42;
    constexpr std::size_t OFFLINE_CRYPTOGRAM_MIN_LONG = 122;
    constexpr std::size_t OFFLINE_CRYPTOGRAM_MAX_LONG = 314;
    constexpr std::size_t OFFLINE_ACK_SIZE            = 8;

    const std::size_t cryptogramSize = offlineCryptogram.size();
    const bool validCryptogramSize =
        cryptogramSize == OFFLINE_CRYPTOGRAM_SHORT || cryptogramSize == OFFLINE_CRYPTOGRAM_MEDIUM ||
        (cryptogramSize >= OFFLINE_CRYPTOGRAM_MIN_LONG && cryptogramSize <= OFFLINE_CRYPTOGRAM_MAX_LONG);

    EXCEPTION_ASSERT_WITH_LOG(validCryptogramSize,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid offline cryptogram length."));

    constexpr std::uint16_t SW_UNKNOWN_PK_INDEX    = 0x6A83;
    constexpr std::uint8_t P2_IMPORT_SETTINGS_ONLY = 0x80;
    constexpr std::uint8_t P2_IMPORT_FULL          = 0x00;
    constexpr std::uint8_t LE_EXPECT_RESPONSE      = 0x00;

    const sam::ApduFormat type = expectResponseData ? sam::ApduFormat::ExtendedWithLe : sam::ApduFormat::Extended;
    
    const unsigned char lc = cryptogramSize > sam::MAX_SECURE_APDU_DATA_SIZE ?
        0x00 : static_cast<unsigned char>(cryptogramSize);
    ByteVector apdu;
    apdu.reserve((expectResponseData ? sam::APDU_HEADER_WITH_LE_SIZE : sam::APDU_HEADER_SIZE) + cryptogramSize);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::ImportCaPk);
    apdu.push_back(0x00);
    apdu.push_back(settingsOnly ? P2_IMPORT_SETTINGS_ONLY : P2_IMPORT_FULL);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), offlineCryptogram.begin(), offlineCryptogram.end());
    if (expectResponseData)
        apdu.push_back(LE_EXPECT_RESPONSE);
    
    const ByteVector response = executeProtectedExchange(apdu, type, sam::EMV_LAYOUT);

    if (sam::parseStatusWord(response) == SW_UNKNOWN_PK_INDEX)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, sam::errorMessage(__func__, "Unknown PK index."));

    validateSuccessResponse(response, __func__);

    if (!expectResponseData)
        return {};

    const ByteVector ack(response.begin(), response.end() - sam::STATUS_WORD_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(ack.size() == OFFLINE_ACK_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid offline acknowledgment length."));

    return ack;
}

void SAMAV3ISO7816Commands::PKI_RemoveCaPk(const ByteVector &rid, unsigned char pkId)
{
    constexpr std::size_t RID_SIZE     = 5;
    constexpr std::size_t PAYLOAD_SIZE = RID_SIZE + 1;

    EXCEPTION_ASSERT_WITH_LOG(rid.size() == RID_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "RID must be exactly 5 bytes."));

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_SIZE + PAYLOAD_SIZE);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::RemoveCaPk);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(PAYLOAD_SIZE));
    apdu.insert(apdu.end(), rid.begin(), rid.end());
    apdu.push_back(pkId);

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Standard, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
}

ByteVector SAMAV3ISO7816Commands::PKI_RemoveCaPkOffline(const ByteVector &offlineCryptogram, bool expectResponseData)
{
    constexpr std::size_t OFFLINE_CRYPTOGRAM_SIZE = 0x1A;
    constexpr std::size_t OFFLINE_ACK_SIZE        = 8;
    constexpr std::uint8_t LE_EXPECT_RESPONSE     = 0x00;

    EXCEPTION_ASSERT_WITH_LOG(offlineCryptogram.size() == OFFLINE_CRYPTOGRAM_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Offline cryptogram must be exactly 26 bytes."));

    ByteVector apdu;
    apdu.reserve((expectResponseData ? sam::APDU_HEADER_WITH_LE_SIZE : sam::APDU_HEADER_SIZE) + OFFLINE_CRYPTOGRAM_SIZE);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::RemoveCaPk);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(OFFLINE_CRYPTOGRAM_SIZE));
    apdu.insert(apdu.end(), offlineCryptogram.begin(), offlineCryptogram.end());
    if (expectResponseData)
        apdu.push_back(LE_EXPECT_RESPONSE);

    const ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Standard, sam::EMV_LAYOUT);

    validateSuccessResponse(response, __func__);

    if (!expectResponseData)
        return {};

    const ByteVector ack(response.begin(), response.end() - sam::STATUS_WORD_SIZE);

    EXCEPTION_ASSERT_WITH_LOG(ack.size() == OFFLINE_ACK_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid offline acknowledgment length."));

    return ack;
}

ByteVector SAMAV3ISO7816Commands::PKI_ExportCaPk(const ByteVector &rid, unsigned char pkId, bool settingsOnly)
{
    constexpr std::size_t RID_SIZE            = 5;
    constexpr std::size_t PAYLOAD_SIZE        = RID_SIZE + 1;
    constexpr std::uint8_t P1_SETTINGS_ONLY   = 0x80;
    constexpr std::uint8_t P1_FULL            = 0x00;
    constexpr std::uint8_t LE_EXPECT_RESPONSE = 0x00;

    EXCEPTION_ASSERT_WITH_LOG(rid.size() == RID_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "RID must be exactly 5 bytes."));

    const unsigned char p1 = settingsOnly ? P1_SETTINGS_ONLY : P1_FULL;

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + PAYLOAD_SIZE);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::ExportCaPk);
    apdu.push_back(p1);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(PAYLOAD_SIZE));
    apdu.insert(apdu.end(), rid.begin(), rid.end());
    apdu.push_back(pkId);
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Standard, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV3ISO7816Commands::PKI_LoadIssuerPk(const ByteVector &rid, unsigned char pkId,
                                                   const ByteVector &issuerPkCert, const ByteVector &issuerPkRemainder,
                                                   unsigned char pkExpLen)
{
    constexpr std::size_t RID_SIZE              = 5;
    constexpr std::size_t MAX_LENGTH_FIELD_SIZE = 0xFF;
    constexpr unsigned char EXPONENT_SHORT_SIZE = 0x01;
    constexpr unsigned char EXPONENT_LONG_SIZE  = 0x03;
    constexpr std::uint8_t LE_EXPECT_RESPONSE   = 0x00;
    constexpr std::size_t RESPONSE_SIZE         = 9;

    EXCEPTION_ASSERT_WITH_LOG(rid.size() == RID_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "RID must be exactly 5 bytes."));

    EXCEPTION_ASSERT_WITH_LOG(!issuerPkCert.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "Issuer public key certificate cannot be empty."));

    EXCEPTION_ASSERT_WITH_LOG(pkExpLen == EXPONENT_SHORT_SIZE || pkExpLen == EXPONENT_LONG_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid public exponent length."));

    EXCEPTION_ASSERT_WITH_LOG(issuerPkCert.size() <= MAX_LENGTH_FIELD_SIZE &&
        issuerPkRemainder.size() <= MAX_LENGTH_FIELD_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Issuer key component too large."));

    const ByteVector pkExp = (pkExpLen == EXPONENT_SHORT_SIZE) ? ByteVector{0x03} : ByteVector{0x01, 0x00, 0x01};

    ByteVector payload;
    payload.reserve(rid.size() + 1u + 1u + issuerPkCert.size() + 1u + issuerPkRemainder.size() + 1u + pkExpLen);
    payload.insert(payload.end(), rid.begin(), rid.end());
    payload.push_back(pkId);
    payload.push_back(static_cast<unsigned char>(issuerPkCert.size()));
    payload.insert(payload.end(), issuerPkCert.begin(), issuerPkCert.end());
    payload.push_back(static_cast<unsigned char>(issuerPkRemainder.size()));
    if (!issuerPkRemainder.empty())
        payload.insert(payload.end(), issuerPkRemainder.begin(), issuerPkRemainder.end());
    payload.push_back(pkExpLen);
    payload.insert(payload.end(), pkExp.begin(), pkExp.end());

    const unsigned char lc =
        payload.size() > sam::MAX_SECURE_APDU_DATA_SIZE ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::LoadIssuerPk);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(response.size() == RESPONSE_SIZE,
                              LibLogicalAccessException, sam::errorMessage(__func__, "Invalid response length."));
    return response;
}

ByteVector SAMAV3ISO7816Commands::PKI_LoadIccPk(const ByteVector &iccPkCert, const ByteVector &iccPkRemainder,
                                                const ByteVector &staticData, unsigned char pkExpLen)
{
    EXCEPTION_ASSERT_WITH_LOG(!iccPkCert.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "ICC public key certificate cannot be empty."));

    constexpr std::size_t MAX_LENGTH_FIELD_SIZE = 0xFF;
    constexpr unsigned char EXPONENT_SHORT_SIZE = 0x01;
    constexpr unsigned char EXPONENT_LONG_SIZE  = 0x03;
    constexpr std::uint8_t LE_EXPECT_RESPONSE   = 0x00;
    constexpr std::size_t RESPONSE_SIZE         = 16;

    EXCEPTION_ASSERT_WITH_LOG(pkExpLen == EXPONENT_SHORT_SIZE || pkExpLen == EXPONENT_LONG_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid public exponent length."));

    EXCEPTION_ASSERT_WITH_LOG(iccPkCert.size() <= MAX_LENGTH_FIELD_SIZE && iccPkRemainder.size() <= MAX_LENGTH_FIELD_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "ICC key component too large."));

    const ByteVector pkExp = (pkExpLen == EXPONENT_SHORT_SIZE) ? ByteVector{0x03} : ByteVector{0x01, 0x00, 0x01};

    ByteVector payload;
    payload.reserve(1u + iccPkCert.size() + 1u + iccPkRemainder.size() + 1u + pkExpLen + staticData.size());
    payload.push_back(static_cast<unsigned char>(iccPkCert.size()));
    payload.insert(payload.end(), iccPkCert.begin(), iccPkCert.end());
    payload.push_back(static_cast<unsigned char>(iccPkRemainder.size()));
    if (!iccPkRemainder.empty())
        payload.insert(payload.end(), iccPkRemainder.begin(), iccPkRemainder.end());
    payload.push_back(pkExpLen);
    payload.insert(payload.end(), pkExp.begin(), pkExp.end());
    if (!staticData.empty())
        payload.insert(payload.end(), staticData.begin(), staticData.end());

    const unsigned char lc =
        payload.size() > sam::MAX_SECURE_APDU_DATA_SIZE ? 0x00 : static_cast<unsigned char>(payload.size());
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + payload.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::LoadIccPk);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), payload.begin(), payload.end());
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(response.size() == RESPONSE_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid response length."));
    return response;
}

ByteVector SAMAV3ISO7816Commands::SAM_RecoverStaticData(const ByteVector &ssad)
{
    constexpr std::size_t MIN_SSAD_SIZE       = 0x40;
    constexpr std::uint8_t LE_EXPECT_RESPONSE = 0x00;
    constexpr std::size_t RESPONSE_SIZE       = 23;

    EXCEPTION_ASSERT_WITH_LOG(!ssad.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "SSAD cannot be empty."));

    EXCEPTION_ASSERT_WITH_LOG(ssad.size() >= MIN_SSAD_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "SSAD length is too small."));

    const unsigned char lc =
        ssad.size() > sam::MAX_SECURE_APDU_DATA_SIZE ? 0x00 : static_cast<unsigned char>(ssad.size());
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + ssad.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::RecoverStaticData);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), ssad.begin(), ssad.end());
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    EXCEPTION_ASSERT_WITH_LOG(response.size() == RESPONSE_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "Invalid response length."));
    return response;
}

ByteVector SAMAV3ISO7816Commands::SAM_RecoverDynamicData(const ByteVector &sdad)
{
    constexpr std::size_t MIN_SDAD_SIZE       = 0x40;
    constexpr std::uint8_t LE_EXPECT_RESPONSE = 0x00;

    EXCEPTION_ASSERT_WITH_LOG(!sdad.empty(),
        LibLogicalAccessException, sam::errorMessage(__func__, "SDAD must not be empty."));

    EXCEPTION_ASSERT_WITH_LOG(sdad.size() >= MIN_SDAD_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "SDAD length is too small."));

    const unsigned char lc =
        sdad.size() > sam::MAX_SECURE_APDU_DATA_SIZE ? 0x00 : static_cast<unsigned char>(sdad.size());
    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + sdad.size());
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::RecoverDynamicData);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(lc);
    apdu.insert(apdu.end(), sdad.begin(), sdad.end());
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::ExtendedWithLe, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

ByteVector SAMAV3ISO7816Commands::SAM_EncipherPIN(const ByteVector &pinBlock, const ByteVector &iccNumber)
{
    constexpr std::size_t PIN_BLOCK_SIZE      = 8;
    constexpr std::size_t ICC_NUMBER_SIZE     = 8;
    constexpr std::size_t PAYLOAD_SIZE        = PIN_BLOCK_SIZE + ICC_NUMBER_SIZE;
    constexpr std::uint8_t LE_EXPECT_RESPONSE = 0x00;

    EXCEPTION_ASSERT_WITH_LOG(pinBlock.size() == PIN_BLOCK_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "PIN block must be exactly 8 bytes."));

    EXCEPTION_ASSERT_WITH_LOG(iccNumber.size() == ICC_NUMBER_SIZE,
        LibLogicalAccessException, sam::errorMessage(__func__, "ICC number must be exactly 8 bytes."));

    ByteVector apdu;
    apdu.reserve(sam::APDU_HEADER_WITH_LE_SIZE + PAYLOAD_SIZE);
    apdu.push_back(d_cla);
    apdu.push_back(sam::ins::emv::EncipherPin);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(static_cast<unsigned char>(PAYLOAD_SIZE));
    apdu.insert(apdu.end(), pinBlock.begin(), pinBlock.end());
    apdu.insert(apdu.end(), iccNumber.begin(), iccNumber.end());
    apdu.push_back(LE_EXPECT_RESPONSE);

    ByteVector response = executeProtectedExchange(apdu, sam::ApduFormat::Standard, sam::EMV_LAYOUT);
    validateSuccessResponse(response, __func__);
    response.resize(response.size() - sam::STATUS_WORD_SIZE);
    return response;
}

}