/**
 * \file rawformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Raw Format.
 */

#ifndef LOGICALACCESS_RAWFORMAT_HPP
#define LOGICALACCESS_RAWFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A Raw format.
     */
    class LIBLOGICALACCESS_API RawFormat : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a raw Format.
         */
        RawFormat();

        /**
         * \brief Destructor.
         *
         * Release the raw Format.
         */
        virtual ~RawFormat();

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
         * \brief Get the field length.
         * \param field The field.
         * \return The field length.
         */
        virtual unsigned int getFieldLength(const string& field) const;

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
         * \brief Get the raw data.
         * \return The raw data.
         */
        std::vector<unsigned char> getRawData() const;

        /**
         * \brief Set the raw data.
         * \param value The raw data.
         */
        void setRawData(std::vector<unsigned char>& data);

        /**
         * \brief Check the current format skeleton with another format.
         * \param format The format to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(boost::shared_ptr<Format> format) const;

        /**
         * \brief The format need user configuration to be use.
         * \return True if it need, false otherwise.
         */
        virtual bool needUserConfigurationToBeUse() const;

    protected:

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
         * \brief The raw data.
         */
        std::vector<unsigned char> d_rawData;
    };
}

#endif /* LOGICALACCESS_RAWFORMAT_HPP */