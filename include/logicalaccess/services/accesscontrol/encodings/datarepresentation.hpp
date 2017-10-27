/**
 * \file datadepresentation.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Data rfepresentation.
 */

#ifndef LOGICALACCESS_DATAREPRESENTATION_HPP
#define LOGICALACCESS_DATAREPRESENTATION_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/encodings/encoding.hpp"
#include "logicalaccess/services/accesscontrol/formats/BitsetStream.hpp"

namespace logicalaccess
{
/**
 * \brief A data representation.
 */
class LIBLOGICALACCESS_API DataRepresentation : public Encoding
{
  public:
    /**
     * \brief Virtual Destructor.
     */
    virtual ~DataRepresentation()
    {
    }

    /**
     * \brief Convert data to the encoding type
     * \param data Data to convert
     * \param dataLengthBytes Length of data to convert in bytes
     * \param dataLengthBits Length of data to convert in bits
     * \param convertedData Data after conversion
     * \param convertedLengthBytes Length of "convertedData" in bytes
     * \return Length after conversion in bits
     */
    virtual BitsetStream convertNumeric(const BitsetStream &datam) = 0;

    /**
     * \brief Convert binary data to the encoding type
     * \param data Data to convert
     * \param dataLengthBytes Length of data to convert in bytes
     * \param dataLengthBits Length of data to convert in bits
     * \param convertedData Data after conversion
     * \param convertedLengthBytes Length of "convertedData" in bytes
     * \return Length after conversion in bits
     */
    virtual BitsetStream convertBinary(const BitsetStream &data) = 0;

    /**
     * \brief Get the length after conversation for a given base length in bits
     * \param lengthBits The unconverted length in bits
     * \return The converted length in bits
     */
    virtual unsigned int convertLength(unsigned int lengthBits) = 0;

    /**
     * \brief Revert data
     * \param data Data to revert
     * \param dataLengthBytes Length of data to convert in bytes
     * \param dataLengthBits Length of data to convert in bits
     * \param convertedData Data after reversion
     * \param convertedLengthBytes Length of "convertedData"
     * \return Length after reversion in bits
     */
    virtual BitsetStream revertNumeric(const BitsetStream &data) = 0;

    /**
     * \brief Revert binary data
     * \param data Data to revert
     * \param dataLengthBytes Length of data to convert in bytes
     * \param dataLengthBits Length of data to convert in bits
     * \param convertedData Data after reversion
     * \param convertedLengthBytes Length of "convertedData"
     * \return Length after reversion in bits
     */
    virtual BitsetStream revertBinary(const BitsetStream &data) = 0;

    /**
     * \brief Create a new Data Representation instance by the encoding type.
     * \param type The encoding type
     * \return The new Data Representation instance, or null if the type is unknown.
     */
    static DataRepresentation *getByEncodingType(EncodingType type);
};
}

#endif /* LOGICALACCESS_DATAREPRESENTATION_HPP */