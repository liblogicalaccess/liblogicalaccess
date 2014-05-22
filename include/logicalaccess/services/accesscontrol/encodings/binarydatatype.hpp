/**
 * \file binarydatatype.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Binary data type.
 */

#ifndef LOGICALACCESS_BINARYDATATYPE_HPP
#define LOGICALACCESS_BINARYDATATYPE_HPP	

#include "logicalaccess/services/accesscontrol/encodings/datatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/encoding.hpp"

#include <string>

using std::string;

namespace logicalaccess
{
	/**
	 * \brief A Binary data type class.
	 */
	class LIBLOGICALACCESS_API BinaryDataType : public DataType
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a BinaryDataType encoder.
			 */
			BinaryDataType();

			/**
			 * \brief Destructor.
			 *
			 * Release the BinaryDataType encoder.
			 */
			~BinaryDataType();

			/**
			 * \brief Get the encoder name
			 * \return The encoder name
			 */
			virtual string getName() const;

			/**
			 * \brief Get the encoder type.
			 * \return The encoder type.
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

		private :

			static void Alg(int s, int* i);

			static void Alg(int* i);

		protected:

	};	
}

#endif /* LOGICALACCESS_BINARYDATATYPE_HPP */
