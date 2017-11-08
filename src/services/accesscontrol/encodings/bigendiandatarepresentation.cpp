/**
 * \file bigendiandatarepresentation.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BigEndian Data Representation.
 */

#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

#ifdef __unix__
#include <cstring>
#endif

namespace logicalaccess
{
BigEndianDataRepresentation::BigEndianDataRepresentation()
{
}

BigEndianDataRepresentation::~BigEndianDataRepresentation()
{
}

std::string BigEndianDataRepresentation::getName() const
{
    return std::string("Big Endian");
}

EncodingType BigEndianDataRepresentation::getType() const
{
    return ET_BIGENDIAN;
}

BitsetStream BigEndianDataRepresentation::convertNumeric(const BitsetStream &data)
{
    BitsetStream convertedData;

#if __BYTE_ORDER != __LITTLE_ENDIAN
    convertedData.concat(data.getData(), 0, data.getBitSize() * 8);
#else

    convertedData = BitHelper::swapBytes(data);
#endif
    return convertedData;
}

BitsetStream BigEndianDataRepresentation::convertBinary(const BitsetStream &data)
{
    BitsetStream convertedData;

    convertedData.concat(data.getData(), 0, data.getBitSize());
    return convertedData;
}

unsigned int BigEndianDataRepresentation::convertLength(unsigned int lengthBits)
{
    return lengthBits;
}

BitsetStream BigEndianDataRepresentation::revertNumeric(const BitsetStream &data)
{
    BitsetStream convertedData;

#if __BYTE_ORDER != __LITTLE_ENDIAN
    convertedData.concat(data.getData(), 0, data.getBitSize());
#else
    convertedData = BitHelper::swapBytes(data);
#endif
    return convertedData;
}

BitsetStream BigEndianDataRepresentation::revertBinary(const BitsetStream &data)
{
    BitsetStream convertedData;
    convertedData.concat(data.getData(), 0, data.getBitSize());
    return convertedData;
}
}