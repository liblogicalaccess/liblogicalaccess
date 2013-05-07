/**
 * \file ChecksumDataField.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Checksum data field. The parity is calculate using other fields.
 */

#ifndef LOGICALACCESS_CHECKSUMDATAFIELD_H
#define LOGICALACCESS_CHECKSUMDATAFIELD_H	

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/DataField.h"


namespace LOGICALACCESS
{
	/**
	 * \brief A checksum data field. Not implemented yet.
	 */
	class LIBLOGICALACCESS_API ChecksumDataField : public DataField
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ChecksumDataField();

			/**
			 * \brief Destructor.
			 */
			virtual ~ChecksumDataField();

			/**
			 * \brief Set the parity value.
			 * \param value The field value.
			 */
			void setValue(const char& value);

			/**
			 * \brief Get the field value.
			 * \return The field value.
			 */
			unsigned char getValue() const;

			/**
			 * \brief Set the bits to use positions to calculate checksum. The sum should be 8-bit factor.
			 * \param positions The bits positions.
			 */
			void setBitsUsePositions(vector<unsigned int> positions);

			/**
			 * \brief Get the bits to use positions to calculate checksum.
			 * \return The bits positions.
			 */
			vector<unsigned int> getBitsUsePositions() const;

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

			vector<size_t> d_bitsUsePositions;
	};	
}

#endif /* LOGICALACCESS_CHECKSUMDATAFIELD_H */
