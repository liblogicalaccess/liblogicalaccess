/**
 * \file dataclockformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DataClock format.
 */

#ifndef LOGICALACCESS_DATACLOCKFORMAT_HPP
#define LOGICALACCESS_DATACLOCKFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

namespace logicalaccess
{
/**
 * \brief A data clock format class.
 */
class LIBLOGICALACCESS_API DataClockFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a data clock Format.
     */
    DataClockFormat();

    /**
     * \brief Destructor.
     *
     * Release the data clock Format.
     */
    virtual ~DataClockFormat();

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
     * \brief Get the Data Clock right parity for a buffer and a location.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \param type The parity type.
     * \param locations The buffer containing all the parity mask location.
     * \param positionLength Number of location contains in the locations buffer.
     * \return The parity.
     */
    static unsigned char getRightParity(const BitsetStream &data, ParityType type,
                                        std::vector<unsigned int> locations);

    /**
     * \brief Get the 1 Data Clock right parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity1(const BitsetStream &data);

    /**
     * \brief Get the 2 Data Clock right parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity2(const BitsetStream &data);

    /**
     * \brief Get the 3 Data Clock right parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity3(const BitsetStream &data);

    /**
     * \brief Get the 4 Data Clock right parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity4(const BitsetStream &data);

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
};
}

#endif /* LOGICALACCESS_DATACLOCKFORMAT_HPP */