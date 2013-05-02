/**
 * \file NoDataRepresentation.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Data Representation which do nothing.
 */

#include "logicalaccess/Services/AccessControl/Encodings/NoDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include <cstring>

namespace LOGICALACCESS
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

	unsigned int NoDataRepresentation::convertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}

		return dataLengthBits;
	}

	unsigned int NoDataRepresentation::convertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}

		return dataLengthBits;
	}

	unsigned int NoDataRepresentation::convertLength(unsigned int lengthBits)
	{
		return lengthBits;
	}

	unsigned int NoDataRepresentation::revertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}

		return dataLengthBits;
	}

	unsigned int NoDataRepresentation::revertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes)
	{
		if (convertedLengthBytes >= dataLengthBytes)
		{
			memset(convertedData, 0x00, convertedLengthBytes);
			memcpy(convertedData, data, dataLengthBytes);				
		}

		return dataLengthBits;
	}
}
