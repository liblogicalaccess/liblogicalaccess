/**
 * \file littleendiandatarepresentation.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief LittleEndian Data Representation.
 */

#include <logicalaccess/services/accesscontrol/encodings/littleendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

#include <cstring>

namespace logicalaccess
{
LittleEndianDataRepresentation::LittleEndianDataRepresentation()
{
}

LittleEndianDataRepresentation::~LittleEndianDataRepresentation()
{
}

std::string LittleEndianDataRepresentation::getName() const
{
    return std::string("Little Endian");
}

EncodingType LittleEndianDataRepresentation::getType() const
{
    return ET_LITTLEENDIAN;
}

BitsetStream LittleEndianDataRepresentation::convertNumeric(const BitsetStream &data)
{
    BitsetStream convertedData;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    convertedData.concat(data.getData(), 0, data.getByteSize() * 8);
#else
    convertedData = BitHelper::swapBytes(data);
#endif
    return convertedData;
}

BitsetStream LittleEndianDataRepresentation::convertBinary(const BitsetStream &data)
{
    return BitHelper::swapBytes(data);
}

unsigned int LittleEndianDataRepresentation::convertLength(unsigned int lengthBits)
{
    return (lengthBits + (((lengthBits % 8) > 0) ? (8 - (lengthBits % 8)) : 0));
}

BitsetStream LittleEndianDataRepresentation::revertNumeric(const BitsetStream &data)
{
    BitsetStream convertedData;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    convertedData.concat(data.getData(), 0, data.getByteSize() * 8);
#else
    convertedData = BitHelper::swapBytes(data);
#endif
    return convertedData;
}

BitsetStream LittleEndianDataRepresentation::revertBinary(const BitsetStream &data)
{
    return BitHelper::swapBytes(data);
}
}