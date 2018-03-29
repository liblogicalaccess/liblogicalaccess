/**
 * \file binarydatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Binary Data field.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
BinaryFieldValue::BinaryFieldValue()
    : Key()
{
    Key::clear();
    d_buf.clear();
}

BinaryFieldValue::BinaryFieldValue(const std::string &str)
    : Key()
{
    d_buf.clear();
    d_buf.resize((str.size() + 2) / 3);
    Key::clear();
    Key::fromString(str);
}

BinaryFieldValue::BinaryFieldValue(const ByteVector &buf)
    : Key()
{
    Key::clear();
    d_buf.clear();

    if (buf.size() != 0)
    {
        d_buf.insert(d_buf.end(), buf.begin(), buf.end());
        d_isEmpty = false;
    }
}

void BinaryFieldValue::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    Key::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void BinaryFieldValue::unSerialize(boost::property_tree::ptree &node)
{
    Key::unSerialize(node);
}

std::string BinaryFieldValue::getDefaultXmlNodeName() const
{
    return "BinaryFieldValue";
}

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
    d_value = BinaryFieldValue(value);
}

ByteVector BinaryDataField::getValue() const
{
    const unsigned char *vdata = d_value.getData();
    return ByteVector(vdata, vdata + d_value.getLength());
}

void BinaryDataField::setPaddingChar(unsigned char padding)
{
    d_padding = padding;
}

unsigned char BinaryDataField::getPaddingChar() const
{
    return d_padding;
}

BitsetStream BinaryDataField::getLinearData(const BitsetStream & /*data*/) const
{
    unsigned int fieldDataLengthBytes = (d_length + 7) / 8;
    ByteVector tmp(d_value.getData(), d_value.getData() + d_value.getLength());

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

    d_value = BinaryFieldValue(paddedBuffer.getData());
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

void BinaryDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    ValueDataField::serialize(node);
    node.put("Value", d_value.getString());
    node.put("Padding", d_padding);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void BinaryDataField::unSerialize(boost::property_tree::ptree &node)
{
    ValueDataField::unSerialize(node);
    d_value   = BinaryFieldValue(node.get_child("Value").get_value<std::string>());
    d_padding = node.get_child("Padding").get_value<unsigned char>();
}

std::string BinaryDataField::getDefaultXmlNodeName() const
{
    return "BinaryDataField";
}
}
