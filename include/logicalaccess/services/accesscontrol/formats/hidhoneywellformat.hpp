/**
 * \file hidhoneywellformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HID Honeywell Format.
 */

#ifndef LOGICALACCESS_HIDHONEYWELLFORMAT_HPP
#define LOGICALACCESS_HIDHONEYWELLFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A HID Honeywell format class.
     */
    class LIBLOGICALACCESS_API HIDHoneywellFormat : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         */
        HIDHoneywellFormat();

        /**
         * \brief Destructor.
         */
        virtual ~HIDHoneywellFormat();

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
         * \brief Get a right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \param rpNo The right parity number (0 to 7).
         * \return The parity.
         */
		static unsigned char getRightParity(const void* data, size_t dataLengthBytes, unsigned char rpNo);

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
	    void getLinearData(void* data, size_t dataLengthBytes) const override;

        /**
         * \brief Set linear data
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         */
	    void setLinearData(const void* data, size_t dataLengthBytes) override;

        /**
         * \brief Get the format linear data in bytes.
         * \param data The data buffer
         * \param dataLengthBytes The data buffer length
         * \return The format linear data length.
         */
	    size_t getFormatLinearData(void* data, size_t dataLengthBytes) const override;

        /**
         * \brief Set the format linear data in bytes.
         * \param data The data buffer
         * \param indexByte The data offset
         */
	    void setFormatLinearData(const void* data, size_t* indexByte) override;

        /**
         * \brief Get the format type.
         * \return The format type.
         */
	    FormatType getType() const override;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
	    void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
	    void unSerialize(boost::property_tree::ptree& node) override;

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

        struct {
            /**
             * \brief The facility code.
             */
            unsigned short int d_facilityCode;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_HIDHONEYWELLFORMAT_HPP */