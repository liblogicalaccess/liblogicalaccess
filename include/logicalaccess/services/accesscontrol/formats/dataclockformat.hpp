/**
 * \file dataclockformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DataClock format.
 */

#ifndef LOGICALACCESS_DATACLOCKFORMAT_HPP
#define LOGICALACCESS_DATACLOCKFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

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
        virtual unsigned int getDataLength() const;

        /**
         * \brief Get the format name
         * \return The format name
         */
        virtual string getName() const;

        /**
         * \brief Get the Data Clock right parity for a buffer and a location.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \param type The parity type.
         * \param locations The buffer containing all the parity mask location.
         * \param positionLength Number of location contains in the locations buffer.
         * \return The parity.
         */
        unsigned char getRightParity(const void* data, size_t dataLengthBytes, ParityType type, unsigned int* locations, size_t positionLength) const;

        /**
         * \brief Get the 1 Data Clock right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity1(const void* data, size_t dataLengthBytes) const;

        /**
         * \brief Get the 2 Data Clock right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity2(const void* data, size_t dataLengthBytes) const;

        /**
         * \brief Get the 3 Data Clock right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity3(const void* data, size_t dataLengthBytes) const;

        /**
         * \brief Get the 4 Data Clock right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity4(const void* data, size_t dataLengthBytes) const;

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
        virtual void getLinearData(void* data, size_t dataLengthBytes) const;

        /**
         * \brief Set linear data
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         */
        virtual void setLinearData(const void* data, size_t dataLengthBytes);

        /**
         * \brief Get the format linear data in bytes.
         * \param data The data buffer
         * \param dataLengthBytes The data buffer length
         * \return The format linear data length.
         */
        virtual size_t getFormatLinearData(void* data, size_t dataLengthBytes) const;

        /**
         * \brief Set the format linear data in bytes.
         * \param data The data buffer
         * \param indexByte The data offset
         */
        virtual void setFormatLinearData(const void* data, size_t* indexByte);

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

    protected:
    };
}

#endif /* LOGICALACCESS_DATACLOCKFORMAT_HPP */