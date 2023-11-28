/**
 * \file wiegand34withfacilityformat.h
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Wiegand34 with facility format class.
 */

#ifndef LOGICALACCESS_WIEGAND34WITHFACILITYFORMAT_HPP
#define LOGICALACCESS_WIEGAND34WITHFACILITYFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/wiegand34format.hpp>

namespace logicalaccess
{
/**
 * \brief A Wiegand 34 with Facility format class.
 */
class LLA_CORE_API Wiegand34WithFacilityFormat : public Wiegand34Format
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a Wiegand 34 With Facility Format.
     */
    Wiegand34WithFacilityFormat();

    /**
     * \brief Destructor.
     *
     * Release the Wiegand 34 With Facility Format.
     */
    virtual ~Wiegand34WithFacilityFormat();

    /**
     * \brief Get the format name
     * \return The format name
     */
    std::string getName() const override;

    /**
     * \brief Get facility code
     */
    unsigned short int getFacilityCode() const;

    /**
     * \brief Set facility code.
     * \param facilityCode The facility code.
     */
    void setFacilityCode(unsigned short int facilityCode);

    /**
     * \brief Get linear data without parity
     * \param data Buffer to be modified. Should be allocated.
     * \param dataLengthBytes Length in byte of data
     */
    BitsetStream getLinearDataWithoutParity() const override;

    /**
     * \brief Write linear data without parity into data buffer
     * \param data Buffer where data will be written.
     * \param dataLengthBytes Length of data in bytes
     */
    void setLinearDataWithoutParity(const ByteVector &data) override;

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

#endif /* LOGICALACCESS_WIEGAND34WITHFACILITYFORMAT_HPP */