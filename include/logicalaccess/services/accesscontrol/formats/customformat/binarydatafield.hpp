/**
 * \file binarydatafield.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Binary data field.
 */

#ifndef LOGICALACCESS_BINARYDATAFIELD_HPP
#define LOGICALACCESS_BINARYDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp>

namespace logicalaccess
{
/**
 * \brief A binary data field.
 */
class LLA_CORE_API BinaryDataField : public ValueDataField
{
  public:
    /**
     * \brief Constructor.
     */
    BinaryDataField();

    /**
     * \brief Destructor.
     */
    virtual ~BinaryDataField();

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    DataFieldType getDFType() const override
    {
        return DFT_BINARY;
    }

    /**
     * \brief Set the field value.
     * \param value The field value.
     */
    void setValue(ByteVector value);

    /**
     * \brief Get the field value.
     * \return The field value.
     */
    ByteVector getValue() const;

    /**
     * \brief Set the padding hex char.
     * \param padding The padding hex char.
     */
    void setPaddingChar(unsigned char padding);

    /**
     * \brief Get the padding hex char.
     * \return The padding hex char.
     */
    unsigned char getPaddingChar() const;

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    BitsetStream getLinearData(const BitsetStream &) const override;

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
    ByteVector d_buf;

    unsigned char d_padding;
};
}

#endif /* LOGICALACCESS_BINARYDATAFIELD_HPP */
