/**
 * \file binarydatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Binary data field.
 */

#ifndef LOGICALACCESS_BINARYDATAFIELD_HPP
#define LOGICALACCESS_BINARYDATAFIELD_HPP

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"
#include "logicalaccess/key.hpp"

namespace logicalaccess
{
    class LIBLOGICALACCESS_API BinaryFieldValue : public Key
    {
    public:
        /**
         * \brief Build an empty Binary field value.
         */
        BinaryFieldValue();

        /**
         * \brief Build a Binary field value given a string representation of it.
         * \param str The string representation.
         */
        BinaryFieldValue(const string& str);

        /**
         * \brief Build a Binary field value given a buffer.
         * \param buf The buffer.
         * \param buflen The buffer length.
         */
        BinaryFieldValue(const void* buf, size_t buflen);

        /**
         * \brief Get the field length.
         * \return The field length.
         */
        inline size_t getLength() const { return d_buf.size(); };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline const unsigned char* getData() const { return &d_buf[0]; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline unsigned char* getData() { return &d_buf[0]; };

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

    private:

        /**
         * \brief The key bytes;
         */
        std::vector<unsigned char> d_buf;
    };

    /**
     * \brief A binary data field.
     */
    class LIBLOGICALACCESS_API BinaryDataField : public ValueDataField
    {
    public:
        /**
         * \brief Constructor.
         */
        BinaryDataField();

        /**
         * \brief Destructor.
         */
        virtual ~BinaryDataField();

        /**
         * \brief Set the field value.
         * \param value The field value.
         */
        void setValue(std::vector<unsigned char> value);

        /**
         * \brief Get the field value.
         * \return The field value.
         */
        std::vector<unsigned char> getValue() const;

        /**
         * \brief Set the padding hex char.
         * \param padding The padding hex char.
         */
        void setPaddingChar(unsigned char padding);

        /**
         * \brief Get the padding hex char.
         * \return The padding hex char.
         */
        unsigned char getPaddingChar() const;

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         * \param pos The first position bit. Will contain the position bit after the field.
         */
        virtual void getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         * \param pos The first position bit. Will contain the position bit after the field.
         */
        virtual void setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos);

        /**
         * \brief Check the current field skeleton with another field.
         * \param field The field to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(std::shared_ptr<DataField> field) const;

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

    protected:

        BinaryFieldValue d_value;

        unsigned char d_padding;
    };
}

#endif /* LOGICALACCESS_BINARYDATAFIELD_HPP */