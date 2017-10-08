/**
 * \file wiegand26format.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand26Format.
 */

#ifndef LOGICALACCESS_WIEGAND26FORMAT_HPP
#define LOGICALACCESS_WIEGAND26FORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/wiegandformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A Wiegand 26 format class.
     */
    class LIBLOGICALACCESS_API Wiegand26Format : public WiegandFormat
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a Wiegand 26 Format.
         */
        Wiegand26Format();

        /**
         * \brief Destructor.
         *
         * Release the Wiegand 26 Format.
         */
        virtual ~Wiegand26Format();

        /**
         * \brief Get facility code byte
         */
        unsigned char getFacilityCode() const;

        /**
         * \brief Set facility code byte
         */
        void setFacilityCode(unsigned char facilityCode);

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
         * \brief Get linear data without parity
         * \param data Buffer to be modified. Should be allocated.
         * \param dataLengthBytes Length in byte of data
         */
	    void getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const override;

        /**
         * \brief Write linear data without parity into data buffer
         * \param data Buffer where data will be written.
         * \param dataLengthBytes Length of data in bytes
         */
	    void setLinearDataWithoutParity(const void* data, size_t dataLengthBytes) override;

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
            unsigned char d_facilityCode;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_WIEGAND26FORMAT_HPP */