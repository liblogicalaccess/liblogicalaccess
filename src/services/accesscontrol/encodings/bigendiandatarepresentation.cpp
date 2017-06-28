/**
 * \file bigendiandatarepresentation.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BigEndian Data Representation.
 */

#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

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

    string BigEndianDataRepresentation::getName() const
    {
        return string("Big Endian");
    }

    EncodingType BigEndianDataRepresentation::getType() const
    {
        return ET_BIGENDIAN;
    }

    unsigned int BigEndianDataRepresentation::convertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

#if __BYTE_ORDER != __LITTLE_ENDIAN

        if (convertedData.getByteSize() >= convertedData.getByteSize())
        {
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size());
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }
        ret = data.getBitSize();
#else
        if (convertedData.getByteSize() != 0)
        {
            BitHelper::swapBytes(convertedData, data);
        }
		ret = data.getBitSize();
#endif
        return ret;
    }

    unsigned int BigEndianDataRepresentation::convertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

        if (convertedData.getByteSize() >= data.getByteSize())
        {
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size());
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }
        ret = data.getBitSize();
        return ret;
    }

    unsigned int BigEndianDataRepresentation::convertLength(unsigned int lengthBits)
    {
        return lengthBits;
    }

    unsigned int BigEndianDataRepresentation::revertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {

#if __BYTE_ORDER != __LITTLE_ENDIAN
        if (convertedData.getSize() >= data.getSize())
        {
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }
#else
        if (convertedData.getBitSize() != 0)
        {
            BitHelper::swapBytes(convertedData, data);
        }
#endif
        return data.getBitSize();
    }

    unsigned int BigEndianDataRepresentation::revertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

        if (convertedData.getByteSize() >= data.getByteSize())
        {
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }
        ret = data.getBitSize();

        return ret;
    }
}