/**
 * \file littleendiandatarepresentation.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief LittleEndian Data Representation.
 */

#include "logicalaccess/services/accesscontrol/encodings/littleendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#include <cstring>

namespace logicalaccess
{
    LittleEndianDataRepresentation::LittleEndianDataRepresentation()
    {
    }

    LittleEndianDataRepresentation::~LittleEndianDataRepresentation()
    {
    }

    string LittleEndianDataRepresentation::getName() const
    {
        return string("Little Endian");
    }

    EncodingType LittleEndianDataRepresentation::getType() const
    {
        return ET_LITTLEENDIAN;
    }

    unsigned int LittleEndianDataRepresentation::convertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

#if __BYTE_ORDER == __LITTLE_ENDIAN
        if (convertedData.getByteSize() >= data.getByteSize())
        {
            //memset(convertedData, 0x00, convertedLengthBytes);
            //memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }
        ret = this->convertLength(data.getBitSize());
#else
        BitHelper::swapBytes(convertedData, data);
        ret = this->convertLength(data.getBitSize());

#endif
        return ret;
    }

    unsigned int LittleEndianDataRepresentation::convertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

        BitHelper::swapBytes(convertedData, data);
        ret = this->convertLength(data.getBitSize());

        return ret;
    }

    unsigned int LittleEndianDataRepresentation::convertLength(unsigned int lengthBits)
    {
        return (lengthBits + (((lengthBits % 8) > 0) ? (8 - (lengthBits % 8)) : 0));
    }

    unsigned int LittleEndianDataRepresentation::revertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

#if __BYTE_ORDER == __LITTLE_ENDIAN
		if (convertedData.getByteSize() >= data.getByteSize())
        {
			//memset(convertedData, 0x00, convertedLengthBytes);
			//memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
		}
#else
		BitHelper::swapBytes(convertedData, data);
		ret = data.getBitSize();
#endif
        return ret;
    }

    unsigned int LittleEndianDataRepresentation::revertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
        unsigned int ret = 0;

		BitHelper::swapBytes(convertedData, data);
		ret = data.getBitSize();

        return ret;
    }
}