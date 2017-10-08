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

	std::string NoDataRepresentation::getName() const
    {
        return std::string("No Representation");
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