/**
 * \file ASCIIDataField.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Ascii data field.
 */

#ifndef LOGICALACCESS_ASCIIDATAFIELD_H
#define LOGICALACCESS_ASCIIDATAFIELD_H	

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/ValueDataField.h"


namespace LOGICALACCESS
{
	/**
	 * \brief A ascii data field.
	 */
	class LIBLOGICALACCESS_API ASCIIDataField : public ValueDataField
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ASCIIDataField();

			/**
			 * \brief Destructor.
			 */
			virtual ~ASCIIDataField();			

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
			virtual bool checkSkeleton(boost::shared_ptr<DataField> field) const;

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

			unsigned char d_padding;
	};	
}

#endif /* LOGICALACCESS_ASCIIDATAFIELD_H */
