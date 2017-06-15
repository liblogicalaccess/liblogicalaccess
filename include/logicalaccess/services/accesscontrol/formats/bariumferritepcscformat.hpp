/**
 * \file bariumferritepcscformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief BariumFerrite PCSC format.
 */

#ifndef LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_H
#define LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_H

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

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
        virtual unsigned int getDataLength() const;

        /**
         * \brief Get the format name
         * \return The format name
         */
        virtual string getName() const;

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
        virtual std::vector<uint8_t> getLinearData() const;

        /**
         * \brief Set linear data
         * \param data Where to get data
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
         * \brief Calculate data checksum.
         * \param data The data to calculate.
         * \param datalen The data length.
         * \return The checksum.
         */
        unsigned char calcChecksum(const BitsetStream& data) const;

    protected:

        struct {
            /**
             * \brief The facility code.
             */
            unsigned short int d_facilityCode;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_BARIUMFERRITEPCSCFORMAT_HPP */