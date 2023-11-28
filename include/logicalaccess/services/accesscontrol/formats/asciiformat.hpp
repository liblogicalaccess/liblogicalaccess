/**
 * \file asciiformat.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ASCII Format.
 */

#ifndef LOGICALACCESS_ASCIIFORMAT_HPP
#define LOGICALACCESS_ASCIIFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

namespace logicalaccess
{
/**
 * \brief A ASCII format.
 */
class LLA_CORE_API ASCIIFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a ASCII Format.
     */
    ASCIIFormat();

    /**
     * \brief Destructor.
     *
     * Release the ASCII Format.
     */
    virtual ~ASCIIFormat();

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
     * \brief Get the ASCII value.
     * \return The ASCII value.
     */
    std::string getASCIIValue() const;

    /**
     * \brief Set the ASCII value.
     * \param value The ASCII value.
     */
    void setASCIIValue(std::string value);

    /**
     * \brief Get the ASCII length.
     * \return The ASCII length.
     */
    unsigned int getASCIILength() const;

    /**
     * \brief Set the ASCII length.
     * \param length The ASCII length.
     */
    void setASCIILength(unsigned int length);

    /**
     * \brief Get the padding char.
     * \return The padding char.
     */
    unsigned char getPadding() const;

    /**
     * \brief Set the padding char.
     * \param padding The padding char.
     */
    void setPadding(unsigned char padding);

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
     * \brief The ASCII value.
     */
    std::string d_asciiValue;

    struct
    {
        /**
         * \brief The total ASCII length.
         */
        unsigned int d_asciiLength;

        /**
         * \brief The padding value.
         */
        unsigned char d_padding;
    } d_formatLinear;
};
}

#endif /* LOGICALACCESS_ASCIIFORMAT_HPP */