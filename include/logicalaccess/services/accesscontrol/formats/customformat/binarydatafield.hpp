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
	    explicit BinaryFieldValue(const std::string& str);

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
	    size_t getLength() const override { return d_buf.size(); }

        /**
         * \brief Get the key data.
         * \return The key data.
         */
	    const unsigned char* getData() const override { return &d_buf[0]; }

        /**
         * \brief Get the key data.
         * \return The key data.
         */
	    unsigned char* getData() override { return &d_buf[0]; }

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

    private:

        /**
         * \brief The key bytes;
         */
        ByteVector d_buf;
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
        void setValue(ByteVector value);

        /**
         * \brief Get the field value.
         * \return The field value.
         */
        ByteVector getValue() const;

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
	    void getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const override;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         * \param pos The first position bit. Will contain the position bit after the field.
         */
	    void setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos) override;

        /**
         * \brief Check the current field skeleton with another field.
         * \param field The field to check.
         * \return True on success, false otherwise.
         */
	    bool checkSkeleton(std::shared_ptr<DataField> field) const override;

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

    protected:

        BinaryFieldValue d_value;

        unsigned char d_padding;
    };
}

#endif /* LOGICALACCESS_BINARYDATAFIELD_HPP */