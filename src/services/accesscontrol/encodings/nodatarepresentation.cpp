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

    BitsetStream NoDataRepresentation::convertNumeric(const BitsetStream& data)
    {
        BitsetStream convertedData;
        convertedData.concat(data.getData(), 0, data.getBitSize());
                        return convertedData;
    }

    BitsetStream NoDataRepresentation::convertBinary(const BitsetStream& data)
    {
        BitsetStream convertedData;
        convertedData.concat(data.getData(), 0, data.getBitSize());
        return convertedData;
    }

    unsigned int NoDataRepresentation::convertLength(unsigned int lengthBits)
    {
        return lengthBits;
    }

    BitsetStream NoDataRepresentation::revertNumeric(const BitsetStream& data)
    {
        BitsetStream convertedData;
        convertedData.concat(data.getData(), 0, data.getBitSize());
        return convertedData;
    }

    BitsetStream NoDataRepresentation::revertBinary(const BitsetStream& data)
    {
        BitsetStream convertedData;
        convertedData.concat(data.getData(), 0, data.getBitSize());
        return convertedData;
    }
}