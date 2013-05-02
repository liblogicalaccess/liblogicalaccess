/**
 * \file StaticFormat.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Static Format.
 */

#ifndef LOGICALACCESS_STATICFORMAT_H
#define LOGICALACCESS_STATICFORMAT_H	

#include "logicalaccess/Services/AccessControl/Formats/Format.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief A static format.
	 */
	class LIBLOGICALACCESS_API StaticFormat : public Format
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a Format.
			 */
			StaticFormat();

			/**
			 * \brief Get the Data Representation for the format.
			 * \return The Data Representation.
			 */
			boost::shared_ptr<DataRepresentation> getDataRepresentation() const;

			/**
			 * \brief Set the Data Representation for the format.
			 * \param encoding The Data Representation.
			 */
			void setDataRepresentation(boost::shared_ptr<DataRepresentation>& encoding);

			/**
			 * \brief Get the Data Type for the format.
			 * \return The Data Type.
			 */
			boost::shared_ptr<DataType> getDataType() const;

			/**
			 * \brief Set the Data Type for the format.
			 * \param encoding The Data Type.
			 */
			void setDataType(boost::shared_ptr<DataType>& encoding);

			/**
			 * \brief Get the UID number.
			 * \return The UID.
			 */
			unsigned long long getUid() const;

			/**
			 * \brief Set the UID number.
			 * \param uid The UID.
			 */
			void setUid(unsigned long long uid);			

			/**
			 * \brief The format need user configuration to be use.
			 * \return True if it need, false otherwise.
			 */
			virtual bool needUserConfigurationToBeUse() const;

			/**
			 * \brief Calculate parity for a block of data.
			 * \param data Data block
			 * \param dataLengthBytes Length of data in bytes
			 * \param parityType Parity type
			 * \param start Bit to start
			 * \param parityLengthBits Length of parity in bits
			 * \return The parity.
			 */
			static unsigned char calculateParity(const void* data, size_t dataLengthBytes, ParityType parityType, size_t start, size_t parityLengthBits);

			/**
			 * \brief Convert a field into the configured DataRepresentation and DataType.
			 * \param data The buffer data that will contains the result.
			 * \param dataLengthBytes The buffer data length.
			 * \param pos The current bit position into the buffer. Will contains the new position.
			 * \param field The field value.
			 * \param fieldlen The field length (in bits).
			 */
			void convertField(void* data, size_t dataLengthBytes, unsigned int* pos, unsigned long long field, unsigned int fieldlen) const;

			/**
			 * \brief Revert a field using the configured DataRepresentation and DataType.
			 * \param data The buffer data that contains the encoded field value.
			 * \param dataLengthBytes The buffer data length.
			 * \param pos The current bit position into the buffer. Will contains the new position.
			 * \param fieldlen The field length (in bits).
			 * \return The field value.
			 */
			unsigned long long revertField(const void* data, size_t dataLengthBytes, unsigned int* pos, unsigned int fieldlen) const;

			/**
			 * \brief Get skeleton linear data.
			 * \param data Where to put data
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual size_t getSkeletonLinearData(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Set skeleton linear data.
			 * \param data Where to get data
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual void setSkeletonLinearData(const void* data, size_t dataLengthBytes);

		protected:			

			/**
			 * \brief Get the format linear data in bytes.
			 * \param data The data buffer
			 * \param dataLengthBytes The data buffer length
			 * \return The format linear data length.
			 */
			virtual size_t getFormatLinearData(void* data, size_t dataLengthBytes) const = 0;

			/**
			 * \brief Get the encoding linear data in bytes (Data Representation and Data Type).
			 * \param data The data buffer
			 * \param dataLengthBytes The data buffer length
			 * \return The encoding linear data length.
			 */
			virtual size_t getEncodingLinearData(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Set the format linear data in bytes.
			 * \param data The data buffer
			 * \param indexByte The data offset
			 */
			virtual void setFormatLinearData(const void* data, size_t* indexByte) = 0;

			/**
			 * \brief Set the encoding linear data in bytes (Data Representation and Data Type).
			 * \param data The data buffer
			 * \param indexByte The data offset
			 */
			virtual void setEncodingLinearData(const void* data, size_t* indexByte);

			/**
			 * \brief The Data Representation.
			 */
			boost::shared_ptr<DataRepresentation> d_dataRepresentation;

			/**
			 * \brief The Data Type.
			 */
			boost::shared_ptr<DataType> d_dataType;

			/**
			 * \brief The UID number.
			 */
			unsigned long long d_uid;

	};	
}

#endif /* LOGICALACCESS_STATICFORMAT_H */
