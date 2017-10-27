/**
 * \file rawformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Raw Format.
 */

#ifndef LOGICALACCESS_RAWFORMAT_HPP
#define LOGICALACCESS_RAWFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
/**
 * \brief A Raw format.
 */
class LIBLOGICALACCESS_API RawFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a raw Format.
     */
    RawFormat();

    /**
     * \brief Destructor.
     *
     * Release the raw Format.
     */
    virtual ~RawFormat();

    /**
     * \brief Get the format length in bits
     */
    unsigned int getDataLength() const override;

    /**
     * \brief Get the format name
     * \return The format name
     */
    std::string getName() const override;

    /**
     * \brief Get the format type.
     * \return The format type.
     */
    FormatType getType() const override;

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

    /**
     * \brief Get the field length.
     * \param field The field.
     * \return The field length.
     */
    unsigned int getFieldLength(const std::string &field) const override;

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     */
    ByteVector getLinearData() const override;

    /**
     * \brief Set linear data
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     */
    void setLinearData(const ByteVector &data) override;

    /**
     * \brief Get the raw data.
     * \return The raw data.
     */
    ByteVector getRawData() const;

    /**
     * \brief Set the raw data.
     * \param value The raw data.
     */
    void setRawData(const ByteVector &data);

    /**
     * \brief Check the current format skeleton with another format.
     * \param format The format to check.
     * \return True on success, false otherwise.
     */
    bool checkSkeleton(std::shared_ptr<Format> format) const override;

    /**
     * \brief The format need user configuration to be use.
     * \return True if it need, false otherwise.
     */
    bool needUserConfigurationToBeUse() const override;

  protected:
    /**
     * \brief Get the format linear data in bytes.
     * \param data The data buffer
     * \param dataLengthBytes The data buffer length
     * \return The format linear data length.
     */
    size_t getFormatLinearData(ByteVector &data) const override;

    /**
     * \brief Set the format linear data in bytes.
     * \param data The data buffer
     * \param indexByte The data offset
     */
    void setFormatLinearData(const ByteVector &data, size_t *indexByte) override;

    /**
     * \brief The raw data.
     */
    ByteVector d_rawData;
};
}

#endif /* LOGICALACCESS_RAWFORMAT_HPP */