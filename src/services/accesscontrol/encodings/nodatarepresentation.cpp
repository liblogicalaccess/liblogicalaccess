/**
 * \file nodatarepresentation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data Representation which do nothing.
 */

#include "logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include <cstring>

namespace logicalaccess
{
    NoDataRepresentation::NoDataRepresentation()
    {
    }

    NoDataRepresentation::~NoDataRepresentation()
    {
    }

    string NoDataRepresentation::getName() const
    {
        return string("No Representation");
    }

    EncodingType NoDataRepresentation::getType() const
    {
        return ET_NOENCODING;
    }

    unsigned int NoDataRepresentation::convertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {
        if (convertedData.getByteSize() >= data.getByteSize())
        {
            //memset(convertedData, 0x00, convertedLengthBytes);
            //memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
        }

		return data.getBitSize();
    }

    unsigned int NoDataRepresentation::convertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
		if (convertedData.getByteSize() >= data.getByteSize())
		{
			//memset(convertedData, 0x00, convertedLengthBytes);
			//memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
		}

		return data.getBitSize();
    }

    unsigned int NoDataRepresentation::convertLength(unsigned int lengthBits)
    {
        return lengthBits;
    }

    unsigned int NoDataRepresentation::revertNumeric(const BitsetStream& data, BitsetStream& convertedData)
    {
		if (convertedData.getByteSize() >= data.getByteSize())
		{
			//memset(convertedData, 0x00, convertedLengthBytes);
			//memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
		}

		return data.getBitSize();
    }

    unsigned int NoDataRepresentation::revertBinary(const BitsetStream& data, BitsetStream& convertedData)
    {
		if (convertedData.getByteSize() >= data.getByteSize())
		{
			//memset(convertedData, 0x00, convertedLengthBytes);
			//memcpy(convertedData, data, dataLengthBytes);
			std::vector<uint8_t> tmp(convertedData.getByteSize());
			std::fill(tmp.begin(), tmp.end(), 0x00);
			convertedData.writeAt(0, tmp, 0, tmp.size() * 8);
			convertedData.writeAt(0, data.getData(), 0, data.getByteSize());
		}

		return data.getBitSize();
    }
}