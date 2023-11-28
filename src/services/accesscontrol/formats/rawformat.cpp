/**
 * \file rawformat.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Raw Format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/rawformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
RawFormat::RawFormat()
    : StaticFormat()
{
    d_dataType.reset(new BinaryDataType());
    d_dataRepresentation.reset(new BigEndianDataRepresentation());

    std::shared_ptr<BinaryDataField> field(new BinaryDataField());
    field->setName("RawData");
    field->setIsIdentifier(true);
    field->setDataLength(RawFormat::getFieldLength(field->getName()));
    field->setPaddingChar(0x00);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
}

RawFormat::~RawFormat()
{
}

unsigned int RawFormat::getDataLength() const
{
    return static_cast<unsigned int>(d_rawData.size()) * 8;
}

std::string RawFormat::getName() const
{
    return std::string("Raw");
}

ByteVector RawFormat::getLinearData() const
{
    std::string ret;
    BitsetStream buf;
    buf.concat(getRawData());

    return buf.getData();
}

void RawFormat::setLinearData(const ByteVector &data)
{
    setRawData(data);
}

size_t RawFormat::getFormatLinearData(ByteVector & /*data*/) const
{
    return 0;
}

void RawFormat::setFormatLinearData(const ByteVector & /*data*/, size_t * /*indexByte*/)
{
}

FormatType RawFormat::getType() const
{
    return FT_RAW;
}

void RawFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("RawData", BufferHelper::getHex(getRawData()));

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void RawFormat::unSerialize(boost::property_tree::ptree &node)
{
    ByteVector rawbuf =
        BufferHelper::fromHexString(node.get_child("RawData").get_value<std::string>());
    setRawData(rawbuf);
}

std::string RawFormat::getDefaultXmlNodeName() const
{
    return "RawFormat";
}

ByteVector RawFormat::getRawData() const
{
    std::shared_ptr<BinaryDataField> field =
        std::dynamic_pointer_cast<BinaryDataField>(getFieldFromName("RawData"));
    return field->getValue();
}

void RawFormat::setRawData(const ByteVector &data)
{
    std::shared_ptr<BinaryDataField> field =
        std::dynamic_pointer_cast<BinaryDataField>(getFieldFromName("RawData"));
    field->setDataLength(static_cast<unsigned int>(data.size() * 8));
    field->setValue(data);
    d_rawData = data;
}

bool RawFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<RawFormat> pFormat = std::dynamic_pointer_cast<RawFormat>(format);
        if (pFormat)
        {
            ret = true;
        }
    }
    return ret;
}

unsigned int RawFormat::getFieldLength(const std::string &field) const
{
    unsigned int length = 0;

    if (field == "RawData")
    {
        length = static_cast<unsigned int>(d_rawData.size() * 8);
    }

    return length;
}

bool RawFormat::needUserConfigurationToBeUse() const
{
    return false;
}
}