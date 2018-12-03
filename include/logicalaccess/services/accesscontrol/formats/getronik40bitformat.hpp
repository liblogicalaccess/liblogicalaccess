/**
 * \file getronik40bitformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Getronik 40-Bit Format.
 */

#ifndef LOGICALACCESS_GETRONIK40BITFORMAT_HPP
#define LOGICALACCESS_GETRONIK40BITFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

namespace logicalaccess
{
/**
 * \brief A Getronik 40-Bit format class.
 */
class LLA_CORE_API Getronik40BitFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     */
    Getronik40BitFormat();

    /**
     * \brief Destructor.
     */
    virtual ~Getronik40BitFormat();

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
     * \brief Get field.
     */
    unsigned short int getField() const;

    /**
     * \brief Set facility code.
     * \param field The field.
     */
    void setField(unsigned short int field);

    /**
     * \brief Get a right parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity(const BitsetStream &data);

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
     * \brief Check the current format skeleton with another format.
     * \param format The format to check.
     * \return True on success, false otherwise.
     */
    bool checkSkeleton(std::shared_ptr<Format> format) const override;

  protected:
    struct
    {
        /**
         * \brief The facility code.
         */
        unsigned short int d_field;
    } d_formatLinear;
};
}

#endif /* LOGICALACCESS_GETRONIK40BITFORMAT_HPP */