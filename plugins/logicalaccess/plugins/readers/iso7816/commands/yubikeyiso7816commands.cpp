/**
 * \file yubikeyiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/yubikeyiso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
YubikeyISO7816Commands::YubikeyISO7816Commands()
    : YubikeyCommands(CMD_YUBIKEYISO7816)
{
}

YubikeyISO7816Commands::YubikeyISO7816Commands(std::string ct)
    : YubikeyCommands(ct)
{
}

YubikeyISO7816Commands::~YubikeyISO7816Commands()
{
}

ByteVector YubikeyISO7816Commands::selectYubikeyOTP()
{
    ByteVector aid({ 0xA0, 0x00, 0x00, 0x05, 0x27, 0x20, 0x01 });
    return getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, P1_SELECT_BY_DFNAME, P2_FIRST_RECORD,
        static_cast<unsigned char>(aid.size()), aid).getData();
}

YubikeySelectResponse YubikeyISO7816Commands::selectYubikeyOATH()
{
    ByteVector aid({ 0xA0, 0x00, 0x00, 0x05, 0x27, 0x21, 0x01 });
    YubikeySelectResponse response;
    
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, P1_SELECT_BY_DFNAME, P2_FIRST_RECORD,
        static_cast<unsigned char>(aid.size()), aid);
    auto tlvs = TLV::parse_tlvs(result.getData(), true);
    
    auto version = TLV::get_child(tlvs, YUBIKEY_TLV_VERSION, true);
    response.version = version->value();
    auto name = TLV::get_child(tlvs, YUBIKEY_TLV_NAME, false);
    if (name != nullptr)
    {
        response.name = name->value();
    }
    auto challenge = TLV::get_child(tlvs, YUBIKEY_TLV_CHALLENGE, false);
    if (challenge != nullptr)
    {
        response.challenge = challenge->value();
    }
    auto algorithm = TLV::get_child(tlvs, YUBIKEY_TLV_ALGORITHM, false);
    if (algorithm != nullptr)
    {
        response.algorithm = static_cast<YubikeyHMAC>(algorithm->value_u1());
    }
    
    return response;
}

ByteVector YubikeyISO7816Commands::otp_getResponse(YubikeySlot slot, const ByteVector& challenge)
{
    return getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_PUT, slot, 0x00,
        static_cast<unsigned char>(challenge.size()), challenge).getData();
}

ByteVector YubikeyISO7816Commands::otp_getSerialNumber()
{
    return getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_PUT, 0x10, 0x00).getData();
}

std::vector<TLVPtr> YubikeyISO7816Commands::otp_getDeviceInfo()
{
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_PUT, 0x13, 0x00);
    return TLV::parse_tlvs(result.getData(), true);
}

bool YubikeyISO7816Commands::otp_queryFIPSMode()
{
    bool isFIPS = false;
    try
    {
        auto res = getISO7816ReaderCardAdapter()->sendAPDUCommand(
            ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_PUT, 0x14, 0x00).getData();
        if (res.size() == 1)
        {
            isFIPS = (res.at(0) == 0x01);
        }
    }
    catch (logicalaccess::CardException& ex)
    {
        // In this context WRONG_P1_P2 simply means that the query command is not present. This behavior can be assumed to mean that the key does not support FIPS mode, and that it does not have FIPS capable firmware.
        if (ex.error_code() != logicalaccess::CardException::WRONG_P1_P2)
        {
            throw ex;
        }
    }
    return isFIPS;
}

void YubikeyISO7816Commands::oath_put(const ByteVector& name, YubikeyHMAC algorithm, uint8_t digits, const ByteVector& key, YubikeyProperty property, const ByteVector& imf)
{
    std::vector<TLVPtr> tlvs;
    auto name_tlv = std::make_shared<TLV>(YUBIKEY_TLV_NAME);
    name_tlv->value(name);
    tlvs.push_back(name_tlv);
    auto key_tlv = std::make_shared<TLV>(YUBIKEY_TLV_KEY);
    ByteVector extkey = key;
    extkey.insert(extkey.begin(), digits);
    extkey.insert(extkey.begin(), static_cast<uint8_t>(algorithm));
    key_tlv->value(extkey);
    tlvs.push_back(key_tlv);
    if (property != PROP_NONE)
    {
        auto prop_tlv = std::make_shared<TLV>(YUBIKEY_TLV_PROPERTY);
        prop_tlv->value(static_cast<uint8_t>(property));
        tlvs.push_back(prop_tlv);
    }
    if (imf.size() > 0)
    {
        auto imf_tlv = std::make_shared<TLV>(YUBIKEY_TLV_IMF);
        imf_tlv->value(imf);
        tlvs.push_back(imf_tlv);
    }
    
    auto data = TLV::value_tlvs(tlvs);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_PUT, 0x00, 0x00,
        static_cast<unsigned char>(data.size()), data);
}

void YubikeyISO7816Commands::oath_delete(const ByteVector& name)
{
    auto name_tlv = std::make_shared<TLV>(YUBIKEY_TLV_NAME);
    name_tlv->value(name);
    auto data = name_tlv->compute();
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_DELETE, 0x00, 0x00,
        static_cast<unsigned char>(data.size()), data);
}

void YubikeyISO7816Commands::oath_setCode(YubikeyHMAC algorithm, const ByteVector& key, const ByteVector& challenge)
{
    std::vector<TLVPtr> tlvs;
    auto key_tlv = std::make_shared<TLV>(YUBIKEY_TLV_KEY);
    ByteVector extkey = key;
    extkey.insert(extkey.begin(), static_cast<uint8_t>(algorithm));
    key_tlv->value(extkey);
    tlvs.push_back(key_tlv);
    auto challenge_tlv = std::make_shared<TLV>(YUBIKEY_TLV_CHALLENGE);
    challenge_tlv->value(challenge);
    tlvs.push_back(challenge_tlv);
    
    auto data = TLV::value_tlvs(tlvs);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_SET_CODE, 0x00, 0x00,
        static_cast<unsigned char>(data.size()), data);
}

std::vector<YubikeyListItem> YubikeyISO7816Commands::oath_list()
{
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_LIST, 0x00, 0x00);
    auto tlvs = TLV::parse_tlvs(result.getData(), true);
    std::vector<YubikeyListItem> namelist;
    for (auto it = tlvs.cbegin(); it != tlvs.cend(); ++it)
    {
        EXCEPTION_ASSERT_WITH_LOG((*it)->tag() == YUBIKEY_TLV_NAME_LIST, LibLogicalAccessException, "Only name list TLV is expected.");
        auto namedata = (*it)->value();
        EXCEPTION_ASSERT_WITH_LOG(namedata.size() > 1, LibLogicalAccessException, "Name list TLV length should be > 1.");
        YubikeyListItem nameitem;
        nameitem.algorithm = static_cast<YubikeyHMAC>(namedata.at(0));
        nameitem.name.insert(nameitem.name.end(), namedata.begin() + 1, namedata.end());
        namelist.push_back(nameitem);
    }
    return namelist;
}

void YubikeyISO7816Commands::oath_reset()
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_RESET, 0xDE, 0xAD);
}

YubikeyCalculateResponse YubikeyISO7816Commands::oath_calculate(const ByteVector& name, const ByteVector& challenge, bool truncate)
{
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_CALCULATE, 0x00, truncate ? 0x01 : 0x00);
    auto tlvs = TLV::parse_tlvs(result.getData(), true);
    YubikeyCalculateResponse response;
    ByteVector rspdata;
    auto fullrsp = TLV::get_child(tlvs, YUBIKEY_TLV_FULL_RESPONSE, false);
    if (fullrsp != nullptr)
    {
        rspdata = fullrsp->value();
    }
    else
    {
        auto truncrsp = TLV::get_child(tlvs, YUBIKEY_TLV_TRUNCATED_RESPONSE, false);
        if (truncrsp != nullptr)
        {
            rspdata = truncrsp->value();
        }
    }
    EXCEPTION_ASSERT_WITH_LOG(rspdata.size() > 1, LibLogicalAccessException, "A response TLV was expected.");
    response.digits = rspdata.at(0);
    response.response.insert(response.response.end(), rspdata.begin() + 1, rspdata.end());
    return response;
}

ByteVector YubikeyISO7816Commands::oath_validate(const ByteVector& challenge, const ByteVector& response)
{
    std::vector<TLVPtr> tlvs;
    auto response_tlv = std::make_shared<TLV>(YUBIKEY_TLV_FULL_RESPONSE);
    response_tlv->value(response);
    tlvs.push_back(response_tlv);
    auto challenge_tlv = std::make_shared<TLV>(YUBIKEY_TLV_CHALLENGE);
    challenge_tlv->value(challenge);
    tlvs.push_back(challenge_tlv);
    
    auto data = TLV::value_tlvs(tlvs);
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_VALIDATE, 0x00, 0x00,
        static_cast<unsigned char>(data.size()), data);
    tlvs = TLV::parse_tlvs(result.getData(), true);
    response_tlv = TLV::get_child(tlvs, YUBIKEY_TLV_FULL_RESPONSE, true);
    return response_tlv->value();
}

std::vector<YubikeyCalculateResponse> YubikeyISO7816Commands::oath_calculateAll(const ByteVector& /*challenge*/, bool /*truncate*/)
{
    // Not sure about the use case for such command. It will remains unimplemented until properly understand.
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not implemented yet.");
}

ByteVector YubikeyISO7816Commands::oath_sendRemainingInstruction()
{
    return getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, YUBIKEY_INS_SEND_REMAINING, 0x00, 0x00).getData();
}
}