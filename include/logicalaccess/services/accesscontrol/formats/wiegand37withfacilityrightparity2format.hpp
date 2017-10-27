/**
 * \file wiegand37withfacilityrightparity2format.h
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand37 with facility and Right Parity 2 format class.
 */

#ifndef LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_HPP
#define LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
/**
 * \brief A Wiegand 37 with Facility Right Parity 2 format class.
 */
class LIBLOGICALACCESS_API Wiegand37WithFacilityRightParity2Format : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a Wiegand 37 With Facility Format Right Parity 2.
     */
    Wiegand37WithFacilityRightParity2Format();

    /**
     * \brief Destructor.
     *
     * Release the Wiegand 37 With Facility Format Right Parity 2.
     */
    virtual ~Wiegand37WithFacilityRightParity2Format();

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
     * \brief Get the left parity for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getLeftParity(const BitsetStream &data);

    /**
     * \brief Get the right parity 1 for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity1(const BitsetStream &data);

    /**
     * \brief Get the right parity 2 for a buffer.
     * \param data The buffer.
     * \param dataLengthBytes The buffer length in bytes.
     * \return The parity.
     */
    static unsigned char getRightParity2(const BitsetStream &data);

    /**
     * \brief Get linear data.
     * \param data Buffer to be modified. Should be allocated.
     * \param dataLengthBytes Length in byte of data
     */
    ByteVector getLinearData() const override;

    /**
     * \brief Set linear data.
     * \param data Buffer where data will be written.
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
        unsigned short int d_facilityCode;
    } d_formatLinear;
};
}

#endif /* LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_HPP */