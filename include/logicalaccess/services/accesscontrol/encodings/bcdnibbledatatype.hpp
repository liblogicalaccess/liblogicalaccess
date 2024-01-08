/**
 * \file bcdnibbledatatype.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief BCD-Nibble data type.
 */

#ifndef LOGICALACCESS_BCDNIBBLEDATATYPE_HPP
#define LOGICALACCESS_BCDNIBBLEDATATYPE_HPP

#include <logicalaccess/services/accesscontrol/encodings/datatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/encoding.hpp>

namespace logicalaccess
{
/**
 * \brief A BCD Nibble data type class.
 */
class LLA_CORE_API BCDNibbleDataType : public DataType
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a BCDNibbleDataType encoder.
     */
    BCDNibbleDataType();

    /**
     * \brief Destructor.
     *
     * Release the BCDNibbleDataType encoder.
     */
    ~BCDNibbleDataType();

    /**
     * \brief Get the encoder name
     * \return The encoder name
     */
    std::string getName() const override;

    /**
     * \brief Get the encoder type.
     * \return The encoder type.
     */
    EncodingType getType() const override;

    /**
     * \brief Convert "data" to the data type
     * \param data Data to convert
     * \param dataLengthBits Length of data to convert in bits
     * \param dataConverted Data after conversion
     * \param dataConvertedLengthBytes Length of "dataConverted" in bytes
     * \return Length of data written in bits
     */
    BitsetStream convert(unsigned long long data, unsigned int dataLengthBits) override;

    /**
     * \brief Revert data type to data
     * \param data Data to revert
     * \param dataLengthBytes Length of "data" in bits
     * \param lengthBits Length of data to revert in bits
     * \return Data after reversion
     */
    unsigned long long revert(BitsetStream &data, unsigned int dataLengthBits) override;
};
}

#endif /* LOGICALACCESS_BCDNIBBLEDATATYPE_HPP */