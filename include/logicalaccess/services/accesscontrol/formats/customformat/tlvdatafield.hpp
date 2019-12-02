/**
 * \file tlvdatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TLV data field.
 */

#ifndef LOGICALACCESS_TLVDATAFIELD_HPP
#define LOGICALACCESS_TLVDATAFIELD_HPP

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"

namespace logicalaccess
{
	/**
	 * \brief A TLV data field.
	 */
	class LIBLOGICALACCESS_API TLVDataField : public ValueDataField
	{
	public:
		/**
		 * \brief Constructor.
		 */
		TLVDataField();

		/**
		 * \brief Constructor.
		 */
		TLVDataField(unsigned char tag);

		/**
		 * \brief Destructor.
		 */
		virtual ~TLVDataField();

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

		void setTag(unsigned char tag);

		unsigned char getTag() const;

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

		std::string d_value;

		unsigned char d_tag;
	};
}

#endif /* LOGICALACCESS_TLVDATAFIELD_HPP */