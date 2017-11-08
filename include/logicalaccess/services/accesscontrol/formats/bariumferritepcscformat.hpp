/**
 * \file bariumferritepcscformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief BariumFerrite PCSC format.
 */

#ifndef LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_H
#define LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_H

#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

namespace logicalaccess
{
/**
 * \brief A Barium Ferrite PCSC format class.
 */
class LIBLOGICALACCESS_API BariumFerritePCSCFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     */
    BariumFerritePCSCFormat();

    /**
     * \brief Destructor.
     */
    virtual ~BariumFerritePCSCFormat();

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
     * \brief Get facility code.
     */
    unsigned short int getFacilityCode() const;

    /**
     * \brief Set facility code.
     * \param facilityCode The facility code.
     */
    void setFacilityCode(unsigned short int facilityCode);

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

    /**
     * \brief Calculate data checksum.
     * \param data The data to calculate.
     * \param datalen The data length.
     * \return The checksum.
     */
    static unsigned char calcChecksum(const ByteVector &data);

  protected:
    struct
    {
        /**
         * \brief The facility code.
         */
        unsigned short int d_facilityCode;
    } d_formatLinear;
};
}

#endif /* LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_HPP */