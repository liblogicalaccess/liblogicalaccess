/**
 * \file stringdatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief String data field.
 */

#ifndef LOGICALACCESS_STRINGDATAFIELD_HPP
#define LOGICALACCESS_STRINGDATAFIELD_HPP

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"

namespace logicalaccess
{
    /**
     * \brief A string data field.
     */
    class LIBLOGICALACCESS_API StringDataField : public ValueDataField
    {
    public:
        /**
         * \brief Constructor.
         */
		StringDataField();

        /**
         * \brief Destructor.
         */
		virtual ~StringDataField();

        /**
         * \brief Set the field value.
         * \param value The field value.
         */
        void setValue(const string& value);

        /**
         * \brief Get the field value.
         * \return The field value.
         */
        string getValue() const;

		/**
		* \brief Set the field value.
		* \param value The field value.
		*/
		void setRawValue(const std::vector<unsigned char>& value);

		/**
		* \brief Get the field value.
		* \return The field value.
		*/
		std::vector<unsigned char> getRawValue() const;

		/**
		* \brief Set the field charset.
		* \param charset The field charset.
		*/
		void setCharset(const string& charset);

		/**
		* \brief Get the field charset.
		* \return The field charset.
		*/
		string getCharset() const;

        /**
         * \brief Set the padding char.
         * \param padding The padding char.
         */
        void setPaddingChar(unsigned char padding);

        /**
         * \brief Get the padding char.
         * \return The padding char.
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

        std::vector<unsigned char> d_value;
        unsigned char d_padding;
		std::string d_charset;
    };
}

#endif /* LOGICALACCESS_STRINGDATAFIELD_HPP */