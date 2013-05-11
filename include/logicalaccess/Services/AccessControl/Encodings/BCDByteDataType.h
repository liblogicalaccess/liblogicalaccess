/**
 * \file BCDByteDataType.h
 * \author Arnaud H. <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCDByteDataType.
 */

#ifndef LOGICALACCESS_BCDBYTEDATATYPE_H
#define LOGICALACCESS_BCDBYTEDATATYPE_H	

#include "logicalaccess/Services/AccessControl/Encodings/DataType.h"
#include "logicalaccess/Services/AccessControl/Encodings/Encoding.h"

#include <string>

using std::string;

namespace logicalaccess
{
	/**
	 * \brief A BCD Byte encoder data type class.
	 */
	class LIBLOGICALACCESS_API BCDByteDataType : public DataType
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a BCDByteDataType encoder.
			 */
			BCDByteDataType();

			/**
			 * \brief Destructor.
			 *
			 * Release the BCDByteDataType encoder.
			 */
			~BCDByteDataType();

			/**
			 * \brief Get the encoder name
			 * \return The encoder name
			 */
			virtual string getName() const;

			/**
			 * \brief Get the encoder type
			 * \return The encoder type
			 */
			virtual EncodingType getType() const;

			/**
			 * \brief Convert "data" to the data type			 
			 * \param data Data to convert
			 * \param dataLengthBits Length of data to convert in bits
			 * \param dataConverted Data after conversion
			 * \param dataConvertedLengthBytes Length of "dataConverted" in bytes
			 * \return Length of data written in bits
			 */
			virtual unsigned int convert(unsigned long long data, unsigned int dataLengthBits, void* dataConverted, size_t dataConvertedLengthBytes);

			/**
			 * \brief Revert data type to data
			 * \param data Data to revert
			 * \param dataLengthBytes Length of "data" in bits
			 * \param lengthBits Length of data to revert in bits
			 * \return Data after reversion
			 */
			virtual unsigned long long revert(void* data, size_t dataLengthBytes, unsigned int lengthBits);

		protected:

	};	
}

#endif /* LOGICALACCESS_BCDBYTEDATATYPE_H */
