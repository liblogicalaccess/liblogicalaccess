/**
 * \file datatype.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Data type.
 */

#ifndef LOGICALACCESS_DATATYPE_HPP
#define LOGICALACCESS_DATATYPE_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/encodings/encoding.hpp"

namespace logicalaccess
{
	/**
	 * \brief Parity type
	 */	
	typedef enum {
		PT_NONE = 0x00,
		PT_EVEN = 0x01,
		PT_ODD = 0x02
	} ParityType;	

	/**
	 * \brief A data type.
	 */
	class LIBLOGICALACCESS_API DataType : public Encoding
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DataType();

			/**
			 * \brief Constructor.
			 */
			virtual ~DataType() {};

			/**
			 * \brief Convert "data" to the data type			 
			 * \param data Data to convert
			 * \param dataLengthBits Length of data to convert in bits
			 * \param dataConverted Data after conversion
			 * \param dataConvertedLengthBytes Length of "dataConverted" in bytes
			 * \return Length of data written in bits
			 */
			virtual unsigned int convert(unsigned long long data, unsigned int dataLengthBits, void* dataConverted, size_t dataConvertedLengthBytes) = 0;

			/**
			 * \brief Revert data type to data
			 * \param data Data to revert
			 * \param dataLengthBytes Length of "data" in bits
			 * \param lengthBits Length of data to revert in bits
			 * \return Data after reversion
			 */
			virtual unsigned long long revert(void* data, size_t dataLengthBytes, unsigned int lengthBits) = 0;

			/**
			 * \brief Create a new Data Type instance by the encoding type.
			 * \param type The encoding type
			 * \return The new Data Type instance, or null if the type is unknown.
			 */
			static DataType* getByEncodingType(EncodingType type);

			static long long pow(int base, unsigned int exp);			

			/**
			 * \brief Set the left parity type of the wiegand format
			 * \param type New parity type.
			 */
			void setLeftParityType(ParityType type);

			/**
			 * \brief Get the left parity type.
			 * \return The left parity type.
			 */
			ParityType getLeftParityType() const;

			/**
			 * \brief Set the right parity type of the wiegand format
			 * \param type New parity type.
			 */
			void setRightParityType(ParityType type);

			/**
			 * \brief Get the right parity type.
			 * \return The right parity type.
			 */
			ParityType getRightParityType() const;

			/**
			 * \brief Add parity to a buffer.
			 * \param leftParity The left parity type.
			 * \param rightParity The right parity type.
			 * \param blocklen The length of a block for calculate parity (in bits).
			 * \param buf The current buffer.
			 * \param buflen The current buffer length (in bits).
			 * \param procbuf The allocated buffer with parity.
			 * \param procbuflen The allocated buffer length (in bits).
			 * \return The buffer with parity length (in bits).
			 */
			static unsigned int addParityToBuffer(ParityType leftParity, ParityType rightParity, unsigned int blocklen, void* buf, unsigned int buflen, void* procbuf, unsigned int procbuflen);

			/**
			 * \brief Remove parity to a buffer.
			 * \param leftParity The left parity type.
			 * \param rightParity The right parity type.
			 * \param blocklen The length of a block for calculate parity (in bits).
			 * \param buf The current buffer with parity.
			 * \param buflen The current buffer length (in bits).
			 * \param procbuf The allocated buffer without parity.
			 * \param procbuflen The allocated buffer length (in bits).
			 * \return The buffer without parity length (in bits).
			 */
			static unsigned int removeParityToBuffer(ParityType leftParity, ParityType rightParity, unsigned int blocklen, void* buf, unsigned int buflen, void* procbuf, unsigned int procbuflen);

			/**
			 * \brief Get the bit data representation type.
			 * \param bitDataRepresentationType The bit data representation type.
			 */
			void setBitDataRepresentationType(EncodingType bitDataRepresentationType);

			/**
			 * \brief Get the bit data representation type.
			 * \return The bit data representation type.
			 */
			EncodingType getBitDataRepresentationType() const;

			/**
			 * \brief Convert a character to the oser bit-sex (Least Significant Bit / Most Significant Bit).
			 * \param c The character.
			 * \return The character in other bit-sex.
			 */
			static unsigned char invertBitSex(unsigned char c, size_t length = 8);


		protected:

			/**
			 * \brief The left parity type.
			 */
			ParityType d_leftParityType;

			/**
			 * \brief The right parity type.
			 */
			ParityType d_rightParityType;

			/**
			 * \brief The bit data representation type.
			 */
			EncodingType d_bitDataRepresentationType;
	};	
}

#endif /* LOGICALACCESS_DATATYPE_HPP */
