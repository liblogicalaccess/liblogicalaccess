/**
 * \file bcdbytedatatype.hpp
 * \author Arnaud H. <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCD-Byte data type.
 */

#ifndef LOGICALACCESS_BCDBYTEDATATYPE_HPP
#define LOGICALACCESS_BCDBYTEDATATYPE_HPP

#include "logicalaccess/services/accesscontrol/encodings/datatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/encoding.hpp"

#include "logicalaccess/services/accesscontrol/formats/BitsetStream.hpp"

namespace logicalaccess
{
/**
 * \brief A BCD Byte encoder data type class.
 */
class LIBLOGICALACCESS_API BCDByteDataType : public DataType
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a BCDByteDataType encoder.
     */
    BCDByteDataType();

    /**
     * \brief Destructor.
     *
     * Release the BCDByteDataType encoder.
     */
    ~BCDByteDataType();

    /**
     * \brief Get the encoder name
     * \return The encoder name
     */
    std::string getName() const override;

    /**
     * \brief Get the encoder type
     * \return The encoder type
     */
    EncodingType getType() const override;

    /**
     * \brief Convert "data" to the data type
     * \param data Data to convert
     * \param dataLengthBits Length of data to convert in bits
     * \param dataConverted Data after conversion
     * \return Length of data written in bits
     */
    BitsetStream convert(unsigned long long data, unsigned int dataLengthBits) override;

    /**
     * \brief Revert data type to data
     * \param data Data to revert
     * \return Data after reversion
     */
    unsigned long long revert(BitsetStream &data, unsigned int dataLengthBits) override;
};
}

#endif /* LOGICALACCESS_BCDBYTEDATATYPE_HPP */