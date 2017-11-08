/**
 * \file staticformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Static Format Base.
 */

#include <cstring>
#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>

namespace logicalaccess
{
StaticFormat::StaticFormat()
    : d_uid(0)
{
}

std::shared_ptr<DataRepresentation> StaticFormat::getDataRepresentation() const
{
    return d_dataRepresentation;
}

void StaticFormat::setDataRepresentation(std::shared_ptr<DataRepresentation> &encoding)
{
    d_dataRepresentation = encoding;
}

std::shared_ptr<DataType> StaticFormat::getDataType() const
{
    return d_dataType;
}

void StaticFormat::setDataType(std::shared_ptr<DataType> &encoding)
{
    d_dataType = encoding;
}

unsigned long long StaticFormat::getUid() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
    return static_cast<unsigned long long>(field->getValue());
}

void StaticFormat::setUid(unsigned long long uid)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
    field->setValue(uid);
    d_uid = uid;
}

unsigned char StaticFormat::calculateParity(const BitsetStream &data,
                                            ParityType parityType, size_t start,
                                            size_t parityLengthBits)
{
    EXCEPTION_ASSERT_WITH_LOG(data.getBitSize() > start, std::out_of_range,
                              "Bit position is out of bounds.");

    unsigned char parity = 0x00;
    for (size_t i = start; i < (start + parityLengthBits) && i < (data.getByteSize() * 8);
         i++)
    {
        parity = (parity & 0x01) ^ ((data.getData()[i / 8] >> (7 - (i % 8))) & 0x01);
    }

    switch (parityType)
    {
    case PT_EVEN: break;

    case PT_ODD: parity = (unsigned char)((~parity) & 0x01); break;

    case PT_NONE: parity = 0x00; break;
    }

    return parity;
}

size_t StaticFormat::getEncodingLinearData(ByteVector &data) const
{
    BitsetStream _data;
    _data.concat(data);

    _data.append(static_cast<unsigned char>(d_dataRepresentation->getType()));
    _data.append(static_cast<unsigned char>(d_dataType->getType()));

    return 2 * 8;
}

void StaticFormat::setEncodingLinearData(const ByteVector &data, size_t *indexByte)
{
    if (data.size() != 0)
    {
        d_dataRepresentation.reset(DataRepresentation::getByEncodingType(
            static_cast<EncodingType>(data[(*indexByte)++])));
        d_dataType.reset(
            DataType::getByEncodingType(static_cast<EncodingType>(data[(*indexByte)++])));
    }
}

bool StaticFormat::needUserConfigurationToBeUse() const
{
    return false;
}

void StaticFormat::convertField(BitsetStream &data, unsigned long long field,
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

unsigned long long StaticFormat::revertField(BitsetStream &data, unsigned int *pos,
                                             unsigned int fieldlen) const
{
    unsigned long long ret = 0;

    const auto extraBitsParity = ((d_dataType->getLeftParityType() != PT_NONE) +
                                  (d_dataType->getRightParityType() != PT_NONE)) *
                                 (fieldlen / d_dataType->getBitDataSize());
    BitsetStream extractData = BitHelper::extract(data, *pos, fieldlen + extraBitsParity);
    if (extractData.getBitSize() > 0)
    {
        BitsetStream revertedData = d_dataRepresentation->revertNumeric(extractData);
        if (revertedData.getBitSize() > 0)
        {
            ret = d_dataType->revert(revertedData, fieldlen);
        }
    }
    (*pos) += fieldlen;

    return ret;
}

size_t StaticFormat::getSkeletonLinearData(ByteVector &data) const
{
    size_t index = 0;

    if (data.size() != 0)
    {
        index += getFormatLinearData(data) * 8;
        index += getEncodingLinearData(data);
    }
    else
    {
        ByteVector getFormat, getEncoding;
        index += getFormatLinearData(getFormat) * 8;
        index += getEncodingLinearData(getEncoding);
    }

    return index;
}

void StaticFormat::setSkeletonLinearData(const ByteVector &data)
{
    size_t indexByte = 0;
    setFormatLinearData(data, &indexByte);
    setEncodingLinearData(data, &indexByte);
}
}