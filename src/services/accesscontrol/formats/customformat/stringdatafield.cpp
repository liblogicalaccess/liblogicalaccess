/**
 * \file stringdatafield.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief String Data field.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
StringDataField::StringDataField()
    : ValueDataField()
    , d_padding(0)
    , d_charset("ascii")
{
    d_length = 0;
}

StringDataField::~StringDataField()
{
}

void StringDataField::setValue(const std::string &value)
{
    d_value = ByteVector(value.begin(), value.end());
}

std::string StringDataField::getValue() const
{
    return std::string(d_value.begin(), d_value.end());
}

void StringDataField::setRawValue(const ByteVector &value)
{
    d_value = value;
}

ByteVector StringDataField::getRawValue() const
{
    return d_value;
}

void StringDataField::setCharset(const std::string &charset)
{
    d_charset = charset;
}

std::string StringDataField::getCharset() const
{
    return d_charset;
}

void StringDataField::setPaddingChar(unsigned char padding)
{
    d_padding = padding;
}

unsigned char StringDataField::getPaddingChar() const
{
    return d_padding;
}

BitsetStream StringDataField::getLinearData(const BitsetStream & /*data*/) const
{
    size_t fieldDataLengthBytes = (d_length + 7) / 8;

    BitsetStream paddedBuffer(d_padding, static_cast<unsigned int>(fieldDataLengthBytes));
    unsigned int copyValueLength =
        static_cast<unsigned int>(d_value.size()) * 8 > d_length
            ? d_length
            : static_cast<unsigned int>(d_value.size()) * 8;
    paddedBuffer.writeAt(0, d_value, 0, copyValueLength);

    BitsetStream dataTmp;
    convertBinaryData(paddedBuffer, d_length, dataTmp);

    return dataTmp;
}

void StringDataField::setLinearData(const ByteVector &data)
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

    d_value = paddedBuffer.getData();
}

bool StringDataField::checkSkeleton(std::shared_ptr<DataField> field) const
{
    bool ret = false;
    if (field)
    {
        std::shared_ptr<StringDataField> pField =
            std::dynamic_pointer_cast<StringDataField>(field);
        if (pField)
        {
            ret = (pField->getDataLength() == getDataLength() &&
                   pField->getPaddingChar() == getPaddingChar() &&
                   pField->getPosition() == getPosition() &&
                   (pField->getIsFixedField() || getValue() == "" ||
                    pField->getValue() == getValue()));
        }
    }

    return ret;
}

void StringDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    ValueDataField::serialize(node);
    node.put("Padding", d_padding);
    node.put("Charset", d_charset);
    std::string strv = getValue();
    if (d_charset != "ascii" && d_charset != "us-ascii" && d_charset != "utf-8")
    {
        strv = BufferHelper::getHex(ByteVector(strv.begin(), strv.end()));
    }
    node.put("Value", strv);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void StringDataField::unSerialize(boost::property_tree::ptree &node)
{
    ValueDataField::unSerialize(node);
    d_padding        = node.get_child("Padding").get_value<unsigned char>();
    d_charset        = node.get_child("Charset").get_value<std::string>();
    std::string strv = node.get_child("Value").get_value<std::string>();
    if (d_charset != "ascii" && d_charset != "us-ascii" && d_charset != "utf-8")
    {
        d_value = BufferHelper::fromHexString(strv);
    }
    else
    {
        setValue(strv);
    }
}

std::string StringDataField::getDefaultXmlNodeName() const
{
    return "StringDataField";
}
}
