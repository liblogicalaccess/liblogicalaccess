/**
 * \file BigEndianDataRepresentation.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief BigEndian Data Representation.
 */

#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#ifdef __linux__
#include <cstring>
#endif

namespace LOGICALACCESS
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

	unsigned int BigEndianDataRepresentation::convertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		#if __BYTE_ORDER != __LITTLE_ENDIAN

			if (convertedLengthBytes >= dataLengthBytes)
			{
				memset(convertedData, 0x00, convertedLengthBytes);
				memcpy(convertedData, data, dataLengthBytes);				
			}
			ret = dataLengthBits;

		#else

			if (convertedData != NULL)
			{
				BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
			}
			ret = dataLengthBits;

		#endif

		return ret;
	}

	unsigned int BigEndianDataRepresentation::convertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}
		ret = dataLengthBits;

		return ret;
	}

	unsigned int BigEndianDataRepresentation::convertLength(unsigned int lengthBits)
	{
		return lengthBits;
	}

	unsigned int BigEndianDataRepresentation::revertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		#if __BYTE_ORDER != __LITTLE_ENDIAN

			if (convertedLengthBytes >= dataLengthBytes)
			{
				memset(convertedData, 0x00, convertedLengthBytes);
				memcpy(convertedData, data, dataLengthBytes);				
			}
			ret = dataLengthBits;

		#else

			if (convertedData != NULL)
			{
				BitHelper::swapBytes(convertedData, convertedLengthBytes, data, dataLengthBytes, dataLengthBits);
			}
			ret = dataLengthBits;

		#endif

		return ret;
	}

	unsigned int BigEndianDataRepresentation::revertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		unsigned int ret = 0;

		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}
		ret = dataLengthBits;

		return ret;
	}
}
