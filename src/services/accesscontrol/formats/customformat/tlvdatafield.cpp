/**
 * \file tlvdatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TLV Data field.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/tlvdatafield.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
TLVDataField::TLVDataField()
    : ValueDataField()
    , d_tag(0)
{
    d_length = 16;
}

TLVDataField::TLVDataField(unsigned char tag)
    : ValueDataField()
    , d_tag(tag)
{
    d_length = 16;
}

TLVDataField::~TLVDataField() {}

void TLVDataField::setTag(unsigned char tag)
{
    d_tag = tag;
}

unsigned char TLVDataField::getTag() const
{
    return d_tag;
}

void TLVDataField::setValue(const std::string &value)
{
    d_length = 16 + value.size() * 8;
    d_value  = value;
}

std::string TLVDataField::getValue() const
{
    return d_value;
}

void TLVDataField::setRawValue(const ByteVector &value)
{
    if (value.size() < 2 || value.size() < static_cast<unsigned int>(2 + value[1]))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The data length is too short.");
    }
    d_tag = value[0];
    setValue(std::string(value.begin() + 2, value.end() + 2 + value[1]));
}

ByteVector TLVDataField::getRawValue() const
{
    auto value = ByteVector(d_value.begin(), d_value.end());
    value.insert(value.begin(), value.size());
    value.insert(value.begin(), d_tag);
    return value;
}

BitsetStream TLVDataField::getLinearData(const BitsetStream & /*data*/) const
{
    BitsetStream buffer;
    buffer.concat(getRawValue());

    BitsetStream dataTmp;
    convertBinaryData(buffer, d_length, dataTmp);

    return dataTmp;
}

void TLVDataField::setLinearData(const ByteVector &data)
{
    BitsetStream _data;
    _data.concat(data);

    if ((_data.getBitSize() - getPosition()) < 16)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The data length is too short.");
    }

    BitsetStream paddedBuffer;

    paddedBuffer.concat(revertBinaryData(_data, getPosition(), d_length));

    setRawValue(paddedBuffer.getData());
}

bool TLVDataField::checkSkeleton(std::shared_ptr<DataField> field) const
{
    bool ret = false;
    if (field)
    {
        std::shared_ptr<TLVDataField> pField =
            std::dynamic_pointer_cast<TLVDataField>(field);
        if (pField)
        {
            ret = (pField->getDataLength() == getDataLength() &&
                   pField->getTag() == getTag() &&
                   (pField->getIsFixedField() || getValue() == "" ||
                    pField->getValue() == getValue()));
        }
    }

    return ret;
}

void TLVDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    ValueDataField::serialize(node);
    node.put("Tag", d_tag);
    node.put("Value", getValue());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void TLVDataField::unSerialize(boost::property_tree::ptree &node)
{
    ValueDataField::unSerialize(node);
    d_tag = node.get_child("Tag").get_value<unsigned char>();
    setValue(node.get_child("Value").get_value<std::string>());
}

std::string TLVDataField::getDefaultXmlNodeName() const
{
    return "TLVDataField";
}
}
