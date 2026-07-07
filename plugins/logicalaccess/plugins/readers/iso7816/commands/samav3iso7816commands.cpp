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
    data.push_back(static_cast<unsigned char>(0x80 | (channel & 0x03)));
    data.push_back(targetKeyno);
    data.push_back(static_cast<unsigned char>(0xff & (changeCounter >> 8)));
    data.push_back(static_cast<unsigned char>(0xff & changeCounter));
    if (targetSamUid.size() > 0)
    {
        p2 |= 0x01;
        data.insert(data.end(), targetSamUid.begin(), targetSamUid.end());
    }
    if (divInput.size() > 0)
    {
        p2 |= 0x02;
        data.insert(data.end(), divInput.begin(), divInput.end());
    }
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        d_cla, 0xE1, keyno, p2, static_cast<unsigned char>(data.size()), data, 0x00);
    return result.getData();
}

void SAMAV3ISO7816Commands::PKI_ImportCaPk(
    const ByteVector &rid, unsigned char pkId, unsigned short set, unsigned char keyNoCEK,
    unsigned char keyVCEK, unsigned char keyNoAEK, unsigned char keyVAEK,
    unsigned char pkExpLen, const ByteVector &pk, const ByteVector &checkSum,
    bool settingsOnly)
{

    if (rid.size() != 5)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportCaPk : RID must be exactly 5 bytes");

    if (settingsOnly && (!pk.empty() || !checkSum.empty()))
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportCaPk : crypto parameters must not be provided when settingsOnly = true");

    if (!settingsOnly)
    {
        if (pk.empty() || pk.size() < 0x40 || pk.size() > 0xF8)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : invalid modulus size");

        if (pkExpLen != 0x01 && pkExpLen != 0x03)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : invalid exponent length");

        if (checkSum.size() != 20)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : checksum must be 20 bytes");
    }

    ByteVector payload;
    payload.insert(payload.end(), rid.begin(), rid.end());
    payload.push_back(pkId);
    payload.push_back((set >> 8) & 0xFF);
    payload.push_back(set & 0xFF);
    payload.push_back(keyNoCEK);
    payload.push_back(keyVCEK);
    payload.push_back(keyNoAEK);
    payload.push_back(keyVAEK);
    if (!settingsOnly)
    {
        payload.push_back(0x01);
        payload.push_back(0x01);
        payload.push_back(static_cast<unsigned char>(pk.size()));
        payload.insert(payload.end(), pk.begin(), pk.end());
        payload.push_back(pkExpLen);
        if (pkExpLen == 0x01)
            payload.push_back(0x03);
        else
            payload.insert(payload.end(), {0x01, 0x00, 0x01});
        payload.insert(payload.end(), checkSum.begin(), checkSum.end());
    }

    size_t offset = 0;
    const bool useSM = !d_sessionKey.empty();

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data, bool first,
                         bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x24);
        apdu.push_back(p1);
        apdu.push_back(settingsOnly ? 0x80 : 0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        return transmit(apdu, first, last, useSM);
    };

    while (offset < payload.size())
    {
        const size_t remaining = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : response too short");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

        if (sw == 0x9000)
            return;

        if (sw == 0x90AF)
            continue;

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : RID limit reached");

        if (sw == 0x6A82)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : unknown RID");

        if (sw == 0x6A83)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPk : unknown PK index");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportCaPk : unexpected status word");
    }
    return;
}

ByteVector SAMAV3ISO7816Commands::PKI_ImportCaPkOffline(const ByteVector &offlineCryptogram,
                                             bool settingsOnly, bool requestAck)
{
    if (offlineCryptogram.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportCaPkOffline : cryptogram empty");

    const bool useSM = !d_sessionKey.empty();

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data, bool first,
                         bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + (requestAck ? 1 : 0));
        apdu.push_back(d_cla);
        apdu.push_back(0x24);
        apdu.push_back(p1);
        apdu.push_back(settingsOnly ? 0x80 : 0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        if (requestAck)
            apdu.push_back(0x00);
        return transmit(apdu, first, last, useSM);
    };

    size_t offset = 0;

    while (offset < offlineCryptogram.size())
    {
        const size_t remaining = offlineCryptogram.size() - offset;
        const size_t frameSize = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(offlineCryptogram.begin() + offset,
                         offlineCryptogram.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;

        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPkOffline : response too short");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

        const ByteVector data(result.begin(), result.end() - 2);

        if (sw == 0x9000)
            return requestAck ? data : ByteVector{};

        if (sw == 0x90AF)
            continue;

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPkOffline : change counter error");

        if (sw == 0x6A83)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ImportCaPkOffline : unknown PK index");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ImportCaPkOffline : unexpected status word");
    }

    return ByteVector{};
}

void SAMAV3ISO7816Commands::PKI_RemoveCaPk(const ByteVector &rid,
                                                 unsigned char pkId)
{
    if (rid.size() != 5)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPk : RID must be exactly 5 bytes");

    const bool useSM = !d_sessionKey.empty();

    ByteVector apdu;
    apdu.reserve(5 + 6);
    apdu.push_back(d_cla);
    apdu.push_back(0x2F);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(0x06);
    apdu.insert(apdu.end(), rid.begin(), rid.end());
    apdu.push_back(pkId);

    const ByteVector result = transmit(apdu, true, true, useSM);

    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPk : response too short");

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    if (sw == 0x9000)
        return;

    if (sw == 0x6A80)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPk : invalid RID");

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "PKI_RemoveCaPk : unexpected status word");
}

ByteVector SAMAV3ISO7816Commands::PKI_RemoveCaPkOffline(const ByteVector &offlineCryptogram,
                                             bool requestAck)
{
    if (offlineCryptogram.size() != 26)
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "PKI_RemoveCaPkOffline : OfflineCryptogram must be 26 bytes");
    const bool useSM = !d_sessionKey.empty();

    ByteVector apdu;
    apdu.reserve(5 + offlineCryptogram.size() + (requestAck ? 1 : 0));
    apdu.push_back(d_cla);
    apdu.push_back(0x2F);
    apdu.push_back(0x00);
    apdu.push_back(0x00);
    apdu.push_back(0x1A);
    apdu.insert(apdu.end(), offlineCryptogram.begin(), offlineCryptogram.end());
    if (requestAck)
        apdu.push_back(0x00);

    const ByteVector result = transmit(apdu, true, true, useSM);

    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPkOffline : response too short");

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    const ByteVector data(result.begin(), result.end() - 2);

    if (sw != 0x9000)
    {
        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_RemoveCaPkOffline: incorrect change counter");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPkOffline: unexpected status word");
    }

    if (!requestAck)
        return {};

    if (data.size() != 8)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_RemoveCaPkOffline : invalid OfflineAck size");

    return data;
}

ByteVector SAMAV3ISO7816Commands::PKI_ExportCaPk(const ByteVector &rid,
                                                 unsigned char pkId, bool settingsOnly)
{

    if (rid.size() != 5)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportCaPk : RID must be exactly 5 bytes");

    const bool useSM = !d_sessionKey.empty();
    const unsigned char p1 = settingsOnly ? 0x80 : 0x00;

    ByteVector payload;
    payload.reserve(rid.size() + 1);
    payload.insert(payload.end(), rid.begin(), rid.end());
    payload.push_back(pkId);

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data, bool first,
                         bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x3D);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        if (!data.empty())
            apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last, useSM);
    };

    ByteVector fullResponse;
    bool firstFrame = true;
    int frames = 0; //TODO : check if frames > 2

    while (true)
    {
        const ByteVector data   = firstFrame ? payload : ByteVector{};
        const bool isLast       = (frames == 1);
        ByteVector result = buildAPDU(p1, data, firstFrame, isLast);
        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportCaPk : response too short");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];
        result.resize(result.size() - 2);

        if (!result.empty())
            fullResponse.insert(fullResponse.end(), result.begin(), result.end());

        frames++;

        if (sw == 0x9000)
            return fullResponse;
        else if (sw == 0x90AF)
        {
            /* if (frames >= 2)
            {
                THROW_EXCEPTION_WITH_LOG(
                    LibLogicalAccessException,
                    "PKI_ExportCaPk : more than 2 frames not supported");
            }*/
            firstFrame = false;
            continue;
        }
        if (sw == 0x6986)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportCaPk : export not allowed");

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_ExportCaPk : invalid RID");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_ExportCaPk : unexpected status word");
    }
}

ByteVector SAMAV3ISO7816Commands::PKI_LoadIssuerPk(
    const ByteVector &rid, unsigned char pkId, const ByteVector &issuerPkCert,
    const ByteVector &issuerPkRemainder, unsigned char pkExpLen)
{
    if (rid.size() != 5)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIssuerPk : RID must be exactly 5 bytes");

    if (issuerPkCert.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIssuerPk : issuerPkCert cannot be empty");

    if (pkExpLen != 0x01 && pkExpLen != 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIssuerPk : pkExpLen must be 0x01 or 0x03");

    const ByteVector pkExp =
        (pkExpLen == 0x01) ? ByteVector{0x03} : ByteVector{0x01, 0x00, 0x01};

    ByteVector payload;
    payload.insert(payload.end(), rid.begin(), rid.end());
    payload.push_back(pkId);
    payload.push_back(static_cast<unsigned char>(issuerPkCert.size()));
    payload.insert(payload.end(), issuerPkCert.begin(), issuerPkCert.end());
    payload.push_back(static_cast<unsigned char>(issuerPkRemainder.size()));
    if (!issuerPkRemainder.empty())
        payload.insert(payload.end(), issuerPkRemainder.begin(),
                           issuerPkRemainder.end());
    payload.push_back(pkExpLen);
    payload.insert(payload.end(), pkExp.begin(), pkExp.end());

    ByteVector fullResponse;
    size_t offset   = 0;
    const bool useSM = !d_sessionKey.empty();

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data,
                         bool first, bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x27);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<uint8_t>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last, useSM);
    };

    while (offset < payload.size())
    {
        const size_t remaining = payload.size() - offset;
        const size_t frameSize  = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_LoadIssuerPk : response too short");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];
        fullResponse.insert(fullResponse.end(), result.begin(), result.end() - 2);

        if (sw == 0x9000)
            return fullResponse;

        if (sw == 0x90AF)
            continue;

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_LoadIssuerPk : invalid RID");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIssuerPk : unexpected status word");
    }

    return fullResponse;
}

ByteVector SAMAV3ISO7816Commands::PKI_LoadIccPk(const ByteVector &iccPkCert,
                                                const ByteVector &iccPkRemainder,
                                                const ByteVector &staticData,
                                                unsigned char pkExpLen)
{

    if (iccPkCert.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIccPk : iccPkCert cannot be empty");

    if (pkExpLen != 0x01 && pkExpLen != 0x03)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIccPk : pkExpLen must be 0x01 or 0x03");

    const ByteVector pkExp =
        (pkExpLen == 0x01) ? ByteVector{0x03} : ByteVector{0x01, 0x00, 0x01};

    ByteVector payload;

    payload.push_back(static_cast<unsigned char>(iccPkCert.size()));
    payload.insert(payload.end(), iccPkCert.begin(), iccPkCert.end());
    payload.push_back(static_cast<unsigned char>(iccPkRemainder.size()));
    if (!iccPkRemainder.empty())
        payload.insert(payload.end(), iccPkRemainder.begin(),
                           iccPkRemainder.end());
    payload.push_back(pkExpLen);
    payload.insert(payload.end(), pkExp.begin(), pkExp.end());
    if (!staticData.empty())
        payload.insert(payload.end(), staticData.begin(), staticData.end());

    ByteVector fullResponse;
    size_t offset   = 0;
    const bool useSM = !d_sessionKey.empty();

    auto buildAPDU  = [&](unsigned char p1, const ByteVector &data,
                         bool first, bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x28);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last, useSM);
    };

    while (offset < payload.size())
    {
        const size_t remaining = payload.size() - offset;
        const size_t frameSize = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(payload.begin() + offset, payload.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_LoadIccPk : response too short");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];
        fullResponse.insert(fullResponse.end(), result.begin(), result.end() - 2);

        if (sw == 0x9000)
            return fullResponse;

        if (sw == 0x90AF)
            continue;

        if (sw == 0x6A80)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "PKI_LoadIccPk : missing required remainder");

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "PKI_LoadIccPk : unexpected status word");
    }
    return fullResponse;
}

ByteVector SAMAV3ISO7816Commands::SAM_RecoverStaticData(const ByteVector &ssad)
{
    if (d_sessionKey.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverStaticData : requires S-mode (no session key)");
    if (ssad.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverStaticData : SSAD cannot be empty");
    if (ssad.size() < 0x40)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverStaticData : SDAD too small");

    ByteVector fullResponse;
    size_t offset = 0;

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data, bool first, bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x29);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last, true);
    };

    while (offset < ssad.size())
    {
        const size_t remaining = ssad.size() - offset;
        const size_t frameSize = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(ssad.begin() + offset, ssad.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "SAM_RecoverStaticData : invalid response");

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

        fullResponse.insert(fullResponse.end(), result.begin(), result.end() - 2);

        if (sw == 0x9000)
            return fullResponse;

        if (sw == 0x90AF)
            continue;

        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverStaticData : unexpected status word");
    }

    return fullResponse;
}

ByteVector SAMAV3ISO7816Commands::SAM_RecoverDynamicData(const ByteVector &sdad)
{
    if (d_sessionKey.empty())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAM_RecoverDynamicData : requires S-mode (no session key)");
    if (sdad.empty())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverDynamicData : SDAD cannot be empty");
    if (sdad.size() < 0x40)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverDynamicData : SDAD too small");

    ByteVector fullResponse;
    size_t offset = 0;

    auto buildAPDU = [&](unsigned char p1, const ByteVector &data,
                         bool first, bool last) -> ByteVector
    {
        ByteVector apdu;
        apdu.reserve(5 + data.size() + 1);
        apdu.push_back(d_cla);
        apdu.push_back(0x2A);
        apdu.push_back(p1);
        apdu.push_back(0x00);
        apdu.push_back(static_cast<unsigned char>(data.size()));
        apdu.insert(apdu.end(), data.begin(), data.end());
        apdu.push_back(0x00);
        return transmit(apdu, first, last, true);
    };

    while (offset < sdad.size())
    {
        const size_t remaining = sdad.size() - offset;
        const size_t frameSize = (std::min)(remaining, sam::MAX_APDU_DATA_SIZE);
        const bool isFirstFrame = (offset == 0);
        const bool isLastFrame  = (remaining <= sam::MAX_APDU_DATA_SIZE);

        ByteVector chunk(sdad.begin() + offset, sdad.begin() + offset + frameSize);

        const unsigned char p1 = isLastFrame ? 0x00 : 0xAF;
        offset += frameSize;

        const ByteVector result = buildAPDU(p1, chunk, isFirstFrame, isLastFrame);

        if (result.size() < 2)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "SAM_RecoverDynamicData : response too short");
        }

        const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

        fullResponse.insert(fullResponse.end(), result.begin(), result.end() - 2);
        
        if (sw == 0x9000)
            return fullResponse;
        else if (sw == 0x90AF)
            continue;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_RecoverDynamicData : unexpected status word");
    }
    return fullResponse;
}

ByteVector SAMAV3ISO7816Commands::SAM_EncipherPIN(const ByteVector &pinBlock,
                                                  const ByteVector &iccNumber)
{
    if (d_sessionKey.empty())
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "SAM_EncipherPIN : requires S-mode (no session key)");
    if (pinBlock.size() != 8)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_EncipherPIN : PIN block must be exactly 8 bytes");
    if (iccNumber.size() != 8)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_EncipherPIN : ICC number must be exactly 8 bytes");
    ByteVector apdu;
    apdu.reserve(5 + 16 + 1);
    apdu = {d_cla, 0x2B, 0x00, 0x00, 0x10};
    apdu.insert(apdu.end(), pinBlock.begin(), pinBlock.end());
    apdu.insert(apdu.end(), iccNumber.begin(), iccNumber.end());
    apdu.push_back(0x00);

    const ByteVector result = transmit(apdu, true, true, true);
    if (result.size() < 2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_EncipherPIN : response too short");

    const uint16_t sw = (result[result.size() - 2] << 8) | result[result.size() - 1];

    if (sw != 0x9000)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM_EncipherPIN : unexpected status word");

    return ByteVector(result.begin(), result.end() - 2);
}


}