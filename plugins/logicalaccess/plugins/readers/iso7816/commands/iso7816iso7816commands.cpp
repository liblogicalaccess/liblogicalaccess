/**
 * \file iso7816iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

#include <cstring>

namespace logicalaccess
{
ISO7816ISO7816Commands::ISO7816ISO7816Commands()
    : ISO7816Commands(CMD_ISO7816ISO7816)
{
}

ISO7816ISO7816Commands::ISO7816ISO7816Commands(std::string ct)
    : ISO7816Commands(ct)
{
}

ISO7816ISO7816Commands::~ISO7816ISO7816Commands() {}

void ISO7816ISO7816Commands::setP1P2(size_t offset, unsigned short efid, unsigned char &p1,
                                     unsigned char &p2)
{
    p1 = 0x00;
    p2 = 0x00;

    if (efid == 0)
    {
        p1 = 0x7f & (offset >> 8);
    }
    else
    {
        p1 = 0x80 | (0x0f & efid);
    }
    p2 = 0xff & offset;
}

ByteVector ISO7816ISO7816Commands::readBinary(size_t length, size_t offset, unsigned short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    ByteVector result =
        (length > 0) ? getISO7816ReaderCardAdapter()->sendAPDUCommand(
                           ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_BINARY, p1, p2,
                           static_cast<unsigned char>(length))
                     : getISO7816ReaderCardAdapter()->sendAPDUCommand(
                           ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_BINARY, p1, p2);

    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::writeBinary(const ByteVector &data, size_t offset,
                                         short unsigned efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_WRITE_BINARY, p1, p2,
        static_cast<unsigned char>(data.size()), data);
}

void ISO7816ISO7816Commands::updateBinary(const ByteVector &data, size_t offset,
                                          unsigned short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_UPDATE_BINARY, p1, p2,
        static_cast<unsigned char>(data.size()), data);
}

void ISO7816ISO7816Commands::eraseBinary(size_t offset, unsigned short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(ISO7816_CLA_ISO_COMPATIBLE,
                                                   ISO7816_INS_ERASE_BINARY, p1, p2);
}

ByteVector ISO7816ISO7816Commands::getData(unsigned short dataObject, size_t /*length*/)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_DATA, (0xff & (dataObject >> 8)),
        (0xff & dataObject));
    return ByteVector(result.begin(), result.end() - 2);
}

ByteVector ISO7816ISO7816Commands::getDataList(const ByteVector &data, size_t length,
                                    unsigned short efid)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_DATA_LIST, (0xff & (efid >> 8)),
        (0xff & efid), static_cast<unsigned char>(data.size()), data);
    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::putData(const ByteVector &data, unsigned short dataObject)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_PUT_DATA, (0xff & (dataObject >> 8)),
        (0xff & dataObject), static_cast<unsigned char>(data.size()), data);
}

ByteVector ISO7816ISO7816Commands::getResponse(unsigned char maxlength)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_RESPONSE, 0x00, 0x00, maxlength);
    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::selectFile(unsigned char p1, unsigned char p2,
                                        const ByteVector &data)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_SELECT_FILE, p1, p2,
        static_cast<unsigned char>(data.size()), data, 0x00);
}

ByteVector ISO7816ISO7816Commands::readRecords(unsigned short fid,
                                               unsigned char start_record,
                                               ISORecords record_number)
{
    unsigned char p1 = start_record;
    unsigned char p2 = record_number & 0x0f;
    if (fid != 0)
    {
        p2 += static_cast<unsigned char>((fid & 0xff) << 3);
    }

    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_READ_RECORDS, p1, p2, 0x00);

    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::appendrecord(const ByteVector &data, unsigned short fid)
{
    unsigned char p1 = 0x00;
    unsigned char p2 = 0x00;
    if (fid != 0)
    {
        p2 += static_cast<unsigned char>((fid & 0xff) << 3);
    }

    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_APPEND_RECORD, p1, p2,
        static_cast<unsigned char>(data.size()), data);
}

ByteVector ISO7816ISO7816Commands::getChallenge(unsigned int length)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GET_CHALLENGE, 0x00, 0x00,
        static_cast<unsigned char>(length));

    if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

void ISO7816ISO7816Commands::externalAuthenticate(unsigned char algorithm,
                                                  bool globalReference,
                                                  unsigned char keyno,
                                                  const ByteVector &data)
{
    unsigned char p2 = keyno & 0x0F;
    if (!globalReference)
    {
        p2 |= 0x80;
    }

    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_EXTERNAL_AUTHENTICATE, algorithm, p2,
        static_cast<unsigned char>(data.size()), data);
}

ByteVector ISO7816ISO7816Commands::externalAuthenticate(unsigned char algorithm,
                                                        bool globalReference,
                                                        unsigned char keyno,
                                                        const ByteVector &data,
                                                        unsigned char le)
{
    unsigned char p2 = keyno & 0x0F;
    if (!globalReference)
    {
        p2 |= 0x80;
    }

    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_EXTERNAL_AUTHENTICATE, algorithm, p2,
        static_cast<unsigned char>(data.size()), data, le);

    if (result.size() > 2 && result[result.size() - 2] == 0x90 &&
        result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

ByteVector ISO7816ISO7816Commands::internalAuthenticate(unsigned char algorithm,
                                                        bool globalReference,
                                                        unsigned char keyno,
                                                        const ByteVector &RPCD2,
                                                        unsigned int length)
{
    unsigned char p2 = keyno & 0x0F;
    if (!globalReference)
    {
        p2 |= 0x80;
    }

    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_INTERNAL_AUTHENTICATE, algorithm, p2,
        static_cast<unsigned char>(RPCD2.size()), RPCD2,
        static_cast<unsigned char>(length));

    if (result.size() > 2 && result[result.size() - 2] == 0x90 &&
        result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

ByteVector ISO7816ISO7816Commands::generalAuthenticate(unsigned char algorithm,
                                                       bool globalReference,
                                                       unsigned char keyno,
                                                       const ByteVector &dataField,
                                                       unsigned char le)
{
    unsigned char p2 = keyno & 0x0F;
    if (!globalReference)
    {
        p2 |= 0x80;
    }

    ByteVector result;

    if (dataField.size() > 0)
    {
        result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
            ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GENERAL_AUTHENTICATE, algorithm, p2,
            static_cast<unsigned char>(dataField.size()), dataField, le);
    }
    else
    {
        result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
            ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_GENERAL_AUTHENTICATE, algorithm, p2,
            le);
    }


    if (result.size() > 2 && result[result.size() - 2] == 0x90 &&
        result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

ByteVector ISO7816ISO7816Commands::generalAuthenticate_challenge(unsigned char algorithm,
                                                                 bool globalReference,
                                                                 unsigned char keyno)
{
    auto tlv_authdata =
        std::make_shared<TLV>(ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA);

    tlv_authdata->value(std::make_shared<TLV>(ISO7816_DATA_OBJECT_CHALLENGE));

	auto result = generalAuthenticate(algorithm, globalReference, keyno, tlv_authdata->compute(), 0x00);
    auto rtlv_authdata = TLV::parse_tlvs(result);
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata.size() > 0, LibLogicalAccessException,
                                "Bad Dynamic Authentication Data TLV response format.");
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata.at(0)->tag() ==
                                  ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA,
                              LibLogicalAccessException, "Wrong TLV tag response.");
    auto rtlv_authdata_childs = TLV::parse_tlvs(rtlv_authdata.at(0)->value());
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata_childs.size() > 0, LibLogicalAccessException,
                              "Bad Challenge TLV response format.");
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata_childs.at(0)->tag() ==
                                  ISO7816_DATA_OBJECT_CHALLENGE,
                              LibLogicalAccessException, "Wrong TLV tag response.");
    return rtlv_authdata_childs.at(0)->value();
}

ByteVector ISO7816ISO7816Commands::generalAuthenticate_response(unsigned char algorithm,
                                                                bool globalReference,
                                                                unsigned char keyno,
                                                                const ByteVector &data)
{
    auto tlv_authdata =
        std::make_shared<TLV>(ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA);
    auto tlv_response = std::make_shared<TLV>(ISO7816_DATA_OBJECT_RESPONSE);
    tlv_response->value(data);

    auto result        = generalAuthenticate(algorithm, globalReference, keyno,
                                      tlv_authdata->compute(), 0x00);
    auto rtlv_authdata = TLV::parse_tlvs(result);
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata.size() > 0, LibLogicalAccessException,
                              "Bad Dynamic Authentication Data TLV response format.");
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata.at(0)->tag() ==
                                  ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA,
                              LibLogicalAccessException, "Wrong TLV tag response.");
    auto rtlv_authdata_childs = TLV::parse_tlvs(rtlv_authdata.at(0)->value());
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata_childs.size() > 0, LibLogicalAccessException,
                              "Bad Challenge TLV response format.");
    EXCEPTION_ASSERT_WITH_LOG(rtlv_authdata_childs.at(0)->tag() ==
                                  ISO7816_DATA_OBJECT_RESPONSE,
                              LibLogicalAccessException, "Wrong TLV tag response.");
    return rtlv_authdata_childs.at(0)->value();
}

void ISO7816ISO7816Commands::removeApplication(const ByteVector &data,
                                               ISORemoveApplicationP1 p1)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_REMOVE_APPLICATION, p1, 0x00, data);
}

ByteVector ISO7816ISO7816Commands::applicationManagementRequest(
    const ByteVector &data, ISOApplicationManagementRequestP1 p1, unsigned char p2)
{
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        ISO7816_CLA_ISO_COMPATIBLE, ISO7816_INS_APPLICATION_MANAGEMENT_REQUEST, p1, p2,
        data);

    return ByteVector(result.begin(), result.end() - 2);
}
} // namespace logicalaccess