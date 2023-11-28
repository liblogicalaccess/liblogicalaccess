/**
 * \file Wiegand35Format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Corporate 1000 format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand35format.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>

#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
Wiegand35Format::Wiegand35Format()
    : StaticFormat()
{
    d_dataType.reset(new BinaryDataType());
    d_dataRepresentation.reset(new BigEndianDataRepresentation());

    d_formatLinear.d_companyCode = 0;

    std::shared_ptr<NumberDataField> uidField(new NumberDataField());
    uidField->setName("Uid");
    uidField->setIsIdentifier(true);
    uidField->setDataLength(20);
    uidField->setDataRepresentation(d_dataRepresentation);
    uidField->setDataType(d_dataType);
    d_fieldList.push_back(uidField);
    std::shared_ptr<NumberDataField> ccField(new NumberDataField());
    ccField->setName("CompanyCode");
    ccField->setDataLength(12);
    ccField->setDataRepresentation(d_dataRepresentation);
    ccField->setDataType(d_dataType);
    d_fieldList.push_back(ccField);
}

Wiegand35Format::~Wiegand35Format()
{
}

unsigned int Wiegand35Format::getDataLength() const
{
    return 35;
}

std::string Wiegand35Format::getName() const
{
    return std::string("Wiegand 35"); // Corporate 1000
}

unsigned short int Wiegand35Format::getCompanyCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("CompanyCode"));
    return static_cast<unsigned short int>(field->getValue());
}

void Wiegand35Format::setCompanyCode(unsigned short int companyCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("CompanyCode"));
    field->setValue(companyCode);
    d_formatLinear.d_companyCode = companyCode;
}

unsigned char Wiegand35Format::getLeftParity1(const BitsetStream &data)
{
    unsigned char parity = 0x00;

    if (data.getByteSize() != 0)
    {
        parity = calculateParity(data, PT_ODD, 1, 34);
    }

    return parity;
}

unsigned char Wiegand35Format::getLeftParity2(const BitsetStream &data)
{
    unsigned char parity = 0x00;

    if (data.getByteSize() != 0)
    {
        std::vector<unsigned int> positions(22);
        positions[0]  = 2;
        positions[1]  = 3;
        positions[2]  = 5;
        positions[3]  = 6;
        positions[4]  = 8;
        positions[5]  = 9;
        positions[6]  = 11;
        positions[7]  = 12;
        positions[8]  = 14;
        positions[9]  = 15;
        positions[10] = 17;
        positions[11] = 18;
        positions[12] = 20;
        positions[13] = 21;
        positions[14] = 23;
        positions[15] = 24;
        positions[16] = 26;
        positions[17] = 27;
        positions[18] = 29;
        positions[19] = 30;
        positions[20] = 32;
        positions[21] = 33;
        parity        = Format::calculateParity(data, PT_EVEN, positions);
    }

    return parity;
}

unsigned char Wiegand35Format::getRightParity(const BitsetStream &data)
{
    unsigned char parity = 0x00;

    if (data.getByteSize() != 0)
    {
        std::vector<unsigned int> positions(22);
        positions[0]  = 1;
        positions[1]  = 2;
        positions[2]  = 4;
        positions[3]  = 5;
        positions[4]  = 7;
        positions[5]  = 8;
        positions[6]  = 10;
        positions[7]  = 11;
        positions[8]  = 13;
        positions[9]  = 14;
        positions[10] = 16;
        positions[11] = 17;
        positions[12] = 19;
        positions[13] = 20;
        positions[14] = 22;
        positions[15] = 23;
        positions[16] = 25;
        positions[17] = 26;
        positions[18] = 28;
        positions[19] = 29;
        positions[20] = 31;
        positions[21] = 32;
        parity        = Format::calculateParity(data, PT_ODD, positions);
    }

    return parity;
}

ByteVector Wiegand35Format::getLinearData() const
{
    BitsetStream data;
    data.append(0x00, 0, 2);

    convertField(data, getCompanyCode(), 12);
    convertField(data, getUid(), 20);

    data.append(0x00, 0, 1);

    if (data.getByteSize() != 0)
    {
        data.writeAt(1, getLeftParity2(data), 7, 1);
        data.writeAt(34, getRightParity(data), 7, 1);
        data.writeAt(0, getLeftParity1(data), 7, 1);
    }
    return data.getData();
}

void Wiegand35Format::setLinearData(const ByteVector &data)
{
    unsigned int pos = 2;
    BitsetStream _data;
    _data.concat(data);

    setCompanyCode((short)revertField(_data, &pos, 12));
    setUid(revertField(_data, &pos, 20));

    if (_data.getByteSize() != 0)
    {
        if (_data.getData().size() * 8 < getDataLength())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
        }

        pos                  = 1;
        unsigned char parity = getLeftParity2(_data);
        if ((unsigned char)((unsigned char)((data)[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Left parity 2 format error.");
        }

        pos    = 34;
        parity = getRightParity(_data);
        if ((unsigned char)((unsigned char)((data)[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Right parity format error.");
        }

        pos    = 0;
        parity = getLeftParity1(_data);
        if ((unsigned char)((unsigned char)((data)[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Left parity 1 format error.");
        }
    }
}

size_t Wiegand35Format::getFormatLinearData(ByteVector &data) const
{
    size_t retLength = sizeof(d_formatLinear);
    data.reserve(retLength);

    if (data.size() >= retLength)
    {
        memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));
    }

    return retLength;
}

void Wiegand35Format::setFormatLinearData(const ByteVector &data, size_t *indexByte)
{
    memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
    (*indexByte) += sizeof(d_formatLinear);

    setCompanyCode(d_formatLinear.d_companyCode);
}

FormatType Wiegand35Format::getType() const
{
    return FT_WIEGAND35;
}

void Wiegand35Format::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("CompanyCode", getCompanyCode());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void Wiegand35Format::unSerialize(boost::property_tree::ptree &node)
{
    setCompanyCode(node.get_child("CompanyCode").get_value<unsigned short>());
    setUid(node.get_child("Uid").get_value<unsigned long long>());
}

std::string Wiegand35Format::getDefaultXmlNodeName() const
{
    return "Wiegand35Format";
}

bool Wiegand35Format::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<Wiegand35Format> pFormat =
            std::dynamic_pointer_cast<Wiegand35Format>(format);
        if (pFormat)
        {
            ret = ((d_formatLinear.d_companyCode == 0 ||
                    d_formatLinear.d_companyCode == pFormat->getCompanyCode()));
        }
    }
    return ret;
}

bool Wiegand35Format::needUserConfigurationToBeUse() const
{
    return false;
}
}