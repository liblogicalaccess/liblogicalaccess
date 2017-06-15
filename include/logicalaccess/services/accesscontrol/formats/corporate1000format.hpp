/**
 * \file corporate1000format.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Corporate 1000 format.
 */

#ifndef LOGICALACCESS_CORPORATE1000FORMAT_HPP
#define LOGICALACCESS_CORPORATE1000FORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A Corporate 1000 format class.
     */
    class LIBLOGICALACCESS_API Corporate1000Format : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a Corporate 1000 format.
         */
        Corporate1000Format();

        /**
         * \brief Destructor.
         *
         * Release the Corporate 1000 format.
         */
        virtual ~Corporate1000Format();

        /**
         * \brief Get the format length in bits
         */
        virtual unsigned int getDataLength() const;

        /**
         * \brief Get the format name
         * \return The format name
         */
        virtual string getName() const;

        /**
         * \brief Get company code.
         */
        unsigned short int getCompanyCode() const;

        /**
         * \brief Set company code.
         * \param facilityCode The company code.
         */
        void setCompanyCode(unsigned short int companyCode);

        /**
         * \brief Get the Corporate 1000 left parity 1 for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getLeftParity1(const BitsetStream& data) const;

        /**
         * \brief Get the Corporate 1000 left parity 2 for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getLeftParity2(const BitsetStream& data) const;

        /**
         * \brief Get the Corporate 1000 right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity(const BitsetStream& data) const;

        /**
         * \brief Get linear data without parity
         * \param data Buffer to be modified. Should be allocated.
         * \param dataLengthBytes Length in byte of data
         */
        virtual std::vector<uint8_t> getLinearData() const;

        /**
         * \brief Write linear data without parity into data buffer
         * \param data Buffer where data will be written.
         * \param dataLengthBytes Length of data in bytes
         */
        virtual void setLinearData(const std::vector<uint8_t>& data);

        /**
         * \brief Get the format linear data in bytes.
         * \param data The data buffer
         * \param dataLengthBytes The data buffer length
         * \return The format linear data length.
         */
        virtual size_t getFormatLinearData(std::vector<uint8_t>& data) const;

        /**
         * \brief Set the format linear data in bytes.
         * \param data The data buffer
         * \param indexByte The data offset
         */
        virtual void setFormatLinearData(const std::vector<uint8_t>& data, size_t* indexByte);

        /**
         * \brief Get the format type.
         * \return The format type.
         */
        virtual FormatType getType() const;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const;

        /**
         * \brief Check the current format skeleton with another format.
         * \param format The format to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(std::shared_ptr<Format> format) const;

        /**
         * \brief The format need user configuration to be use.
         * \return True if it need, false otherwise.
         */
        virtual bool needUserConfigurationToBeUse() const;

    protected:

        struct {
            /**
             * \brief The company code.
             */
            unsigned short int d_companyCode;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_CORPORATE1000FORMAT_HPP */