/**
 * \file asciiformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ASCII Format.
 */

#ifndef LOGICALACCESS_ASCIIFORMAT_HPP
#define LOGICALACCESS_ASCIIFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A ASCII format.
     */
    class LIBLOGICALACCESS_API ASCIIFormat : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a ASCII Format.
         */
        ASCIIFormat();

        /**
         * \brief Destructor.
         *
         * Release the ASCII Format.
         */
        virtual ~ASCIIFormat();

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
         * \brief Get the ASCII value.
         * \return The ASCII value.
         */
        string getASCIIValue();

        /**
         * \brief Set the ASCII value.
         * \param value The ASCII value.
         */
        void setASCIIValue(string value);

        /**
         * \brief Get the ASCII length.
         * \return The ASCII length.
         */
        unsigned int getASCIILength() const;

        /**
         * \brief Set the ASCII length.
         * \param length The ASCII length.
         */
        void setASCIILength(unsigned int length);

        /**
         * \brief Get the padding char.
         * \return The padding char.
         */
        unsigned char getPadding();

        /**
         * \brief Set the padding char.
         * \param padding The padding char.
         */
        void setPadding(unsigned char padding);

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
         * \brief The ASCII value.
         */
        string d_asciiValue;

        struct {
            /**
             * \brief The total ASCII length.
             */
            unsigned int d_asciiLength;

            /**
             * \brief The padding value.
             */
            unsigned char d_padding;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_ASCIIFORMAT_HPP */