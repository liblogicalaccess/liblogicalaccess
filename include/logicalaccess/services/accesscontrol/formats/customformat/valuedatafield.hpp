/**
 * \file valuedatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Value data field.
 */

#ifndef LOGICALACCESS_VALUEDATAFIELD_HPP
#define LOGICALACCESS_VALUEDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp>
#include <logicalaccess/services/accesscontrol/encodings/datarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/encodings/datatype.hpp>

namespace logicalaccess
{
/**
 * \brief A value data field base class.
 */
class LIBLOGICALACCESS_API ValueDataField : public DataField
{
  public:
    /**
     * \brief Constructor.
     */
    ValueDataField();

    /**
     * \brief Destructor.
     */
    virtual ~ValueDataField();

    /**
     * \brief Set data length in bits.
     * \param length The data length in bits.
     */
    virtual void setDataLength(unsigned int length);

    /**
     * \brief Get the Data Representation for the field.
     * \return The Data Representation.
     */
    std::shared_ptr<DataRepresentation> getDataRepresentation() const;

    /**
     * \brief Set the Data Representation for the field.
     * \param encoding The Data Representation.
     */
    void setDataRepresentation(std::shared_ptr<DataRepresentation> &encoding);

    /**
     * \brief Get the Data Type for the field.
     * \return The Data Type.
     */
    std::shared_ptr<DataType> getDataType() const;

    /**
     * \brief Set the Data Type for the field.
     * \param encoding The Data Type.
     */
    void setDataType(std::shared_ptr<DataType> &encoding);

    /**
     * \brief Set if the field is a fixed field.
     * \return isFixed True if the field is a fixed field, false otherwise.
     */
    void setIsFixedField(bool isFixed);

    /**
     * \brief Get if the field is a fixed field.
     * \return True if the field is a fixed field, false otherwise.
     */
    bool getIsFixedField() const;

    /**
     * \brief Set if the field is used as identifier.
     * \return isValue True if the field is used as identifier, false otherwise.
     */
    void setIsIdentifier(bool isIdentifier);

    /**
     * \brief Get if the field is used as identifier.
     * \return True if the field is used as identifier, false otherwise.
     */
    bool getIsIdentifier() const;

    /**
     * \brief Convert a numeric data into the configured DataRepresentation and DataType.
     * \param data The buffer data that will contains the result.
     * \param dataLengthBytes The buffer data length.
     * \param pos The current bit position into the buffer. Will contains the new
     * position.
     * \param field The numeric data value.
     * \param fieldlen The field length (in bits).
     */
    void convertNumericData(BitsetStream &data, unsigned long long field,
                            unsigned int fieldlen) const;

    /**
     * \brief Convert a binary data into the configured DataRepresentation.
     * \param data The buffer data that contains the read buffer.
     * \param dataLengthBytes The buffer data length.
     * \param pos The current bit position into the buffer. Will contains the new
     * position.
     * \param fieldlen The field length (in bits).
     * \param convertedData The buffer data that will contains the result.
     * \param convertedDataLengthBytes The buffer data length.
     */
    void convertBinaryData(const BitsetStream &data, unsigned int fieldlen,
                           BitsetStream &convertedData) const;

    /**
     * \brief Revert a numeric data using the configured DataRepresentation and DataType.
     * \param data The buffer data that contains the encoded field value.
     * \param dataLengthBytes The buffer data length.
     * \param pos The current bit position into the buffer. Will contains the new
     * position.
     * \param fieldlen The field length (in bits).
     * \return The numeric data value.
     */
    unsigned long long revertNumericData(const BitsetStream &data, unsigned int pos,
                                         unsigned int fieldlen) const;

    /**
     * \brief Revert a binary data using the configured DataRepresentation.
     * \param data The buffer data that contains the encoded buffer.
     * \param dataLengthBytes The buffer data length.
     * \param pos The current bit position into the buffer. Will contains the new
     * position.
     * \param fieldlen The field length (in bits).
     * \param revertedData The buffer data that will contains the result.
     * \param revertedDataLengthBytes The buffer data length.
     */
    BitsetStream revertBinaryData(const BitsetStream &data, unsigned int pos,
                                  unsigned int fieldlen) const;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

  protected:
    /**
     * \brief The Data Representation.
     */
    std::shared_ptr<DataRepresentation> d_dataRepresentation;

    /**
     * \brief The Data Type.
     */
    std::shared_ptr<DataType> d_dataType;

    /**
     * \brief The field is a value field.
     */
    bool d_isFixedField;

    /**
     * \brief The field is used as identifier.
     */
    bool d_isIdentifier;
};
}

#endif /* LOGICALACCESS_VALUEDATAFIELD_HPP */