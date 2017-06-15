/**
 * \file getronik40bitformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Getronik 40-Bit Format.
 */

#ifndef LOGICALACCESS_GETRONIK40BITFORMAT_HPP
#define LOGICALACCESS_GETRONIK40BITFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

namespace logicalaccess
{
    /**
     * \brief A Getronik 40-Bit format class.
     */
    class LIBLOGICALACCESS_API Getronik40BitFormat : public StaticFormat
    {
    public:
        /**
         * \brief Constructor.
         */
        Getronik40BitFormat();

        /**
         * \brief Destructor.
         */
        virtual ~Getronik40BitFormat();

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
         * \brief Get field.
         */
        unsigned short int getField() const;

        /**
         * \brief Set facility code.
         * \param field The field.
         */
        void setField(unsigned short int field);

        /**
         * \brief Get a right parity for a buffer.
         * \param data The buffer.
         * \param dataLengthBytes The buffer length in bytes.
         * \return The parity.
         */
        unsigned char getRightParity(const BitsetStream& data) const;

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

    protected:

        struct {
            /**
             * \brief The facility code.
             */
            unsigned short int d_field;
        } d_formatLinear;
    };
}

#endif /* LOGICALACCESS_GETRONIK40BITFORMAT_HPP */