/**
 * \file ParityDataField.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Parity data field. The parity is calculate using other fields.
 */

#ifndef LOGICALACCESS_PARITYDATAFIELD_H
#define LOGICALACCESS_PARITYDATAFIELD_H	

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/DataField.h"
#include "logicalaccess/Services/AccessControl/Encodings/DataType.h"


namespace LOGICALACCESS
{
	/**
	 * \brief A parity data field.
	 */
	class LIBLOGICALACCESS_API ParityDataField : public DataField
	{
		public:
			/**
			 * \brief Constructor.
			 */
			ParityDataField();

			/**
			 * \brief Destructor.
			 */
			virtual ~ParityDataField();

			/**
			 * \brief Set the field position in bits.
			 * \param position The field position in bits.
			 */
			virtual void setPosition(unsigned int position);

			/**
			 * \brief Set the parity type.
			 * \param type The parity type.
			 */
			void setParityType(ParityType type);

			/**
			 * \brief Get the parity type.
			 * \return The parity type.
			 */
			ParityType getParityType() const;

			/**
			 * \brief Set the bits to use positions to calculate parity.
			 * \param positions The bits positions.
			 */
			void setBitsUsePositions(std::vector<unsigned int> positions);

			/**
			 * \brief Get the bits to use positions to calculate parity.
			 * \return The bits positions.
			 */
			std::vector<unsigned int> getBitsUsePositions() const;

			/**
			 * \brief Check the field dependecy with another one.
			 * \param field The field to check with.
			 * \return True if the field is dependent, false otherwise.
			 */
			bool checkFieldDependecy(boost::shared_ptr<DataField> field);

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

			ParityType d_parityType;

			std::vector<unsigned int> d_bitsUsePositions;
	};	
}

#endif /* LOGICALACCESS_PARITYDATAFIELD_H */
