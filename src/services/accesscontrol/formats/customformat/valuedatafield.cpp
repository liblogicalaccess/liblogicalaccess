/**
 * \file valuedatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Value Data field.
 */

#include <logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/myexception.hpp>

#include <cstring>
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
ValueDataField::ValueDataField()
    : DataField()
{
    d_isFixedField = false;
    d_isIdentifier = false;
    d_dataRepresentation.reset(new BigEndianDataRepresentation());
    d_dataType.reset(new BinaryDataType());
}

ValueDataField::~ValueDataField()
{
}

void ValueDataField::setDataLength(unsigned int length)
{
    d_length = length;
}

std::shared_ptr<DataRepresentation> ValueDataField::getDataRepresentation() const
{
    return d_dataRepresentation;
}

void ValueDataField::setDataRepresentation(std::shared_ptr<DataRepresentation> &encoding)
{
    d_dataRepresentation = encoding;
}

std::shared_ptr<DataType> ValueDataField::getDataType() const
{
    return d_dataType;
}

void ValueDataField::setDataType(const std::shared_ptr<DataType> &encoding)
{
    d_dataType = encoding;
}

void ValueDataField::setIsFixedField(bool isFixed)
{
    d_isFixedField = isFixed;
}

bool ValueDataField::getIsFixedField() const
{
    return d_isFixedField;
}

void ValueDataField::setIsIdentifier(bool isIdentifier)
{
    d_isIdentifier = isIdentifier;
}

bool ValueDataField::getIsIdentifier() const
{
    return d_isIdentifier;
}

void ValueDataField::convertNumericData(BitsetStream &data, unsigned long long field,
                                        unsigned int fieldlen) const
{
    const BitsetStream convertedDataTypeData = d_dataType->convert(field, fieldlen);
    BitsetStream bitConvertNum =
        d_dataRepresentation->convertNumeric(convertedDataTypeData);
    const auto extraBitsParity = ((d_dataType->getLeftParityType() != PT_NONE) +
                                  (d_dataType->getRightParityType() != PT_NONE)) *
                                 (fieldlen / d_dataType->getBitDataSize());
    data.concat(bitConvertNum.getData(), 0, fieldlen + extraBitsParity);
}

void ValueDataField::convertBinaryData(const BitsetStream &data,
                                       unsigned int /*fieldlen*/,
                                       BitsetStream &convertedData) const
{
    BitsetStream convertedDataRepresentationData =
        d_dataRepresentation->convertBinary(data);
    convertedData.concat(convertedDataRepresentationData.getData(), 0,
                         convertedDataRepresentationData.getBitSize());
}

unsigned long long ValueDataField::revertNumericData(const BitsetStream &data,
                                                     unsigned int pos,
                                                     unsigned int fieldlen) const
{
    unsigned long long ret = 0;


    BitsetStream extractData = BitHelper::extract(data, pos, fieldlen);

    if (extractData.getBitSize() > 0)
    {
        BitsetStream revertedData = d_dataRepresentation->revertNumeric(extractData);
        if (revertedData.getBitSize() > 0)
        {
            ret = d_dataType->revert(revertedData, fieldlen);
        }
    }
    return ret;
}

BitsetStream ValueDataField::revertBinaryData(const BitsetStream &data, unsigned int pos,
                                              unsigned int fieldlen) const
{
    BitsetStream revertedData;
    BitsetStream extractData = BitHelper::extract(data, pos, fieldlen);
    if (extractData.getBitSize() > 0)
    {
        return d_dataRepresentation->revertBinary(extractData);
    }

    return revertedData;
}

void ValueDataField::serialize(boost::property_tree::ptree &node)
{
    DataField::serialize(node);
    node.put("IsFixedField", d_isFixedField);
    node.put("IsIdentifier", d_isIdentifier);
    node.put("DataRepresentation", d_dataRepresentation->getType());
    node.put("DataType", d_dataType->getType());
    node.put("Length", d_length);
}

void ValueDataField::unSerialize(boost::property_tree::ptree &node)
{
    DataField::unSerialize(node);
    d_isFixedField = node.get_child("IsFixedField").get_value<bool>();
    d_isIdentifier = node.get_child("IsIdentifier").get_value<bool>();
    d_dataRepresentation.reset(
        DataRepresentation::getByEncodingType(static_cast<EncodingType>(
            node.get_child("DataRepresentation").get_value<unsigned int>())));
    d_dataType.reset(DataType::getByEncodingType(
        static_cast<EncodingType>(node.get_child("DataType").get_value<unsigned int>())));
    d_length = node.get_child("Length").get_value<unsigned int>();
}
}
