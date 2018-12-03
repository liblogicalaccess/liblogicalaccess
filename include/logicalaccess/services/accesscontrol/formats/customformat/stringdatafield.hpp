/**
 * \file stringdatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief String data field.
 */

#ifndef LOGICALACCESS_STRINGDATAFIELD_HPP
#define LOGICALACCESS_STRINGDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp>

namespace logicalaccess
{
/**
 * \brief A string data field.
 */
class LLA_CORE_API StringDataField : public ValueDataField
{
  public:
    /**
     * \brief Constructor.
     */
    StringDataField();

    /**
     * \brief Destructor.
     */
    virtual ~StringDataField();

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    DataFieldType getDFType() const override
    {
        return DFT_STRING;
    }

    /**
     * \brief Set the field value.
     * \param value The field value.
     */
    void setValue(const std::string &value);

    /**
     * \brief Get the field value.
     * \return The field value.
     */
    std::string getValue() const;

    /**
    * \brief Set the field value.
    * \param value The field value.
    */
    void setRawValue(const ByteVector &value);

    /**
    * \brief Get the field value.
    * \return The field value.
    */
    ByteVector getRawValue() const;

    /**
    * \brief Set the field charset.
    * \param charset The field charset.
    */
    void setCharset(const std::string &charset);

    /**
    * \brief Get the field charset.
    * \return The field charset.
    */
    std::string getCharset() const;

    /**
     * \brief Set the padding char.
     * \param padding The padding char.
     */
    void setPaddingChar(unsigned char padding);

    /**
     * \brief Get the padding char.
     * \return The padding char.
     */
    unsigned char getPaddingChar() const;

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    BitsetStream getLinearData(const BitsetStream &data) const override;

    /**
     * \brief Set linear data.
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    void setLinearData(const ByteVector &data) override;

    /**
     * \brief Check the current field skeleton with another field.
     * \param field The field to check.
     * \return True on success, false otherwise.
     */
    bool checkSkeleton(std::shared_ptr<DataField> field) const override;

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

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

  protected:
    ByteVector d_value;
    unsigned char d_padding;
    std::string d_charset;
};
}

#endif /* LOGICALACCESS_STRINGDATAFIELD_HPP */