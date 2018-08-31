/**
 * \file checksumdatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Checksum data field. The parity is calculate using other fields.
 */

#ifndef LOGICALACCESS_CHECKSUMDATAFIELD_HPP
#define LOGICALACCESS_CHECKSUMDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp>

namespace logicalaccess
{
/**
 * \brief A checksum data field. Not implemented yet.
 */
class LIBLOGICALACCESS_API ChecksumDataField : public DataField
{
  public:
    /**
     * \brief Constructor.
     */
    ChecksumDataField();

    /**
     * \brief Destructor.
     */
    virtual ~ChecksumDataField();

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    DataFieldType getDFType() const override
    {
        return DFT_CHECKSUM;
    }

    /**
     * \brief Set the parity value.
     * \param value The field value.
     */
    void setValue(const char &/*value*/)
    {
        throw std::runtime_error("Unimplemented");
    }

    /**
     * \brief Get the field value.
     * \return The field value.
     */
    unsigned char getValue() const
    {
        throw std::runtime_error("Unimplemented");
    }

    /**
     * \brief Set the bits to use positions to calculate checksum. The sum should be 8-bit
     * factor.
     * \param positions The bits positions.
     */
    void setBitsUsePositions(ByteVector /*positions*/)
    {
        throw std::runtime_error("Unimplemented");
    }

    /**
     * \brief Get the bits to use positions to calculate checksum.
     * \return The bits positions.
     */
    ByteVector getBitsUsePositions() const
    {
        throw std::runtime_error("Unimplemented");
    }

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    virtual void getLinearData(void *data, size_t dataLengthBytes,
                               unsigned int *pos) const;

    /**
     * \brief Set linear data.
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    virtual void setLinearData(const void *data, size_t dataLengthBytes,
                               unsigned int *pos);

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

    std::vector<size_t> d_bitsUsePositions;
};
}

#endif /* LOGICALACCESS_CHECKSUMDATAFIELD_HPP */