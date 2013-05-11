/**
 * \file DataRepresentation.h
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief DataRepresentation.
 */

#ifndef LOGICALACCESS_DATAREPRESENTATION_H
#define LOGICALACCESS_DATAREPRESENTATION_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/Services/AccessControl/Encodings/Encoding.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	/**
	 * \brief A data representation.
	 */
	class LIBLOGICALACCESS_API DataRepresentation : public Encoding
	{
		public:
			/**
			 * \brief Convert data to the encoding type
			 * \param data Data to convert
			 * \param dataLengthBytes Length of data to convert in bytes
			 * \param dataLengthBits Length of data to convert in bits
			 * \param convertedData Data after conversion
			 * \param convertedLengthBytes Length of "convertedData" in bytes			 
			 * \return Length after conversion in bits
			 */
			virtual unsigned int convertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes) = 0;

			/**
			 * \brief Convert binary data to the encoding type
			 * \param data Data to convert
			 * \param dataLengthBytes Length of data to convert in bytes
			 * \param dataLengthBits Length of data to convert in bits
			 * \param convertedData Data after conversion
			 * \param convertedLengthBytes Length of "convertedData" in bytes			 
			 * \return Length after conversion in bits
			 */
			virtual unsigned int convertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes) = 0;

			/**
			 * \brief Get the length after conversation for a given base length in bits
			 * \param lengthBits The unconverted length in bits
			 * \return The converted length in bits
			 */
			virtual unsigned int convertLength(unsigned int lengthBits) = 0;

			/**
			 * \brief Revert data
			 * \param data Data to revert
			 * \param dataLengthBytes Length of data to convert in bytes
			 * \param dataLengthBits Length of data to convert in bits
			 * \param convertedData Data after reversion
			 * \param convertedLengthBytes Length of "convertedData"
			 * \return Length after reversion in bits			 
			 */
			virtual unsigned int revertNumeric(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes) = 0;

			/**
			 * \brief Revert binary data
			 * \param data Data to revert
			 * \param dataLengthBytes Length of data to convert in bytes
			 * \param dataLengthBits Length of data to convert in bits
			 * \param convertedData Data after reversion
			 * \param convertedLengthBytes Length of "convertedData"
			 * \return Length after reversion in bits			 
			 */
			virtual unsigned int revertBinary(const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, void* convertedData, size_t convertedLengthBytes) = 0;

			/**
			 * \brief Create a new Data Representation instance by the encoding type.
			 * \param type The encoding type
			 * \return The new Data Representation instance, or null if the type is unknown.
			 */
			static DataRepresentation* getByEncodingType(EncodingType type);
	};	
}

#endif /* LOGICALACCESS_DATAREPRESENTATION_H */
