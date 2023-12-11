/**
 * \file binarydatafield.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Binary Data field.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
BinaryDataField::BinaryDataField()
    : ValueDataField()
{
    d_length  = 0;
    d_padding = 0x00;
}

BinaryDataField::~BinaryDataField()
{
}

void BinaryDataField::setValue(ByteVector value)
{
    d_buf = value;
}

ByteVector BinaryDataField::getValue() const
{
    return d_buf;
}

void BinaryDataField::setPaddingChar(unsigned char padding)
{
    d_padding = padding;
}

unsigned char BinaryDataField::getPaddingChar() const
{
    return d_padding;
}

BitsetStream BinaryDataField::getLinearData(const BitsetStream &) const
{
    unsigned int fieldDataLengthBytes = (d_length + 7) / 8;
    ByteVector tmp(d_buf);

    BitsetStream paddedBuffer(d_padding, fieldDataLengthBytes);
    unsigned int copyValueLength = static_cast<unsigned int>(tmp.size()) * 8 > d_length
                                       ? d_length
                                       : static_cast<unsigned int>(tmp.size()) * 8;
    paddedBuffer.writeAt(0, tmp, 0, copyValueLength);

    BitsetStream dataTmp;
    convertBinaryData(paddedBuffer, d_length, dataTmp);

    return dataTmp;
}

void BinaryDataField::setLinearData(const ByteVector &data)
{
    BitsetStream _data;
    _data.concat(data);

    if (_data.getBitSize() < (d_length + getPosition()))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The data length is too short.");
    }

    BitsetStream paddedBuffer;

    paddedBuffer.concat(revertBinaryData(_data, getPosition(), d_length));

    d_buf = paddedBuffer.getData();
}

bool BinaryDataField::checkSkeleton(std::shared_ptr<DataField> field) const
{
    bool ret = false;
    if (field)
    {
        std::shared_ptr<BinaryDataField> pField =
            std::dynamic_pointer_cast<BinaryDataField>(field);
        if (pField)
        {
            ret = (pField->getDataLength() == getDataLength() &&
                   pField->getPaddingChar() == getPaddingChar() &&
                   pField->getPosition() == getPosition() &&
                   (pField->getIsFixedField() || getValue().size() == 0 ||
                    pField->getValue() == getValue()));
        }
    }

    return ret;
}

std::string byteVectorToString(const ByteVector& bv)
{
    std::ostringstream oss;
    oss << std::setfill('0');

    for (const auto& byte : bv)
    {
        if (&byte != &bv[0])
        {
            oss << " ";
        }

        oss << std::setw(2) << std::hex << static_cast<unsigned int>(byte);
    }

    return oss.str();
}

ByteVector byteVectorFromString(const std::string& str)
{
    ByteVector bv((str.size() + 2) / 3);

    std::istringstream iss(str);
    unsigned int tmp;

    for (auto& byte : bv)
    {
        if (!iss.good())
        {
            // nothing better to do AFAIK
            return bv;
        }

        iss >> std::hex >> tmp;
        byte = static_cast<ByteVector::value_type>(tmp);
    }

    return bv;
}

void BinaryDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    ValueDataField::serialize(node);
    node.put("Value", byteVectorToString(d_buf));
    node.put("Padding", d_padding);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void BinaryDataField::unSerialize(boost::property_tree::ptree &node)
{
    ValueDataField::unSerialize(node);
    d_buf     = byteVectorFromString(node.get_child("Value").get_value<std::string>());
    d_padding = node.get_child("Padding").get_value<unsigned char>();
}

std::string BinaryDataField::getDefaultXmlNodeName() const
{
    return "BinaryDataField";
}
}
