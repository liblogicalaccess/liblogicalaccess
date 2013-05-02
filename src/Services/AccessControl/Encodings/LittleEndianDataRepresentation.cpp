/**
 * \file LittleEndianDataRepresentation.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>
 * \brief LittleEndian Data Representation.
 */

#include "logicalaccess/Services/AccessControl/Encodings/LittleEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include <cstring>

namespace LOGICALACCESS
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

	unsigned int LittleEndianDataRepresentation::convertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		#if __BYTE_ORDER == __LITTLE_ENDIAN

			if (convertedLengthBytes >= dataLengthBytes)
			{
				memset(convertedData, 0x00, convertedLengthBytes);
				memcpy(convertedData, data, dataLengthBytes);				
			}
			ret = convertLength(dataLengthBits);

		#else

			BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
			ret = convertLength(dataLengthBits);

		#endif

		return ret;
	}

	unsigned int LittleEndianDataRepresentation::convertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
		ret = convertLength(dataLengthBits);

		return ret;
	}

	unsigned int LittleEndianDataRepresentation::convertLength(unsigned int lengthBits)
	{
		return (lengthBits + (((lengthBits % 8) > 0) ? (8 - (lengthBits % 8)) : 0));
	}

	unsigned int LittleEndianDataRepresentation::revertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		#if __BYTE_ORDER == __LITTLE_ENDIAN

			if (convertedLengthBytes >= dataLengthBytes)
			{
				memset(convertedData, 0x00, convertedLengthBytes);
				memcpy(convertedData, data, dataLengthBytes);				
			}
			ret = dataLengthBits;

		#else

			BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
			ret = dataLengthBits;

		#endif

		return ret;
	}

	unsigned int LittleEndianDataRepresentation::revertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
		ret = dataLengthBits;

		return ret;
	}
}
