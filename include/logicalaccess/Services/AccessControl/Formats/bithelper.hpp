/**
 * \file bithelper.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Bit helper.
 */

#ifndef LOGICALACCESS_BITHELPER_HPP
#define LOGICALACCESS_BITHELPER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"

#ifdef LINUX
#include <wintypes.h>
#endif

namespace logicalaccess
{
	/**
	 * \brief A Bit Helper.
	 */
	class LIBLOGICALACCESS_API BitHelper
	{
		public:

			/**
			 * \brief Line "data" buffer in "linedData" with new size (linedDataLength)
			 * \param linedData Buffer to be written
			 * \param linedDataLengthBytes Length of "linedData" in bytes
			 * \param data Buffer to be readed
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 * \return Length of lined data (in bits)
			 */
			static unsigned int align(void* linedData, size_t linedDataLengthBytes,
								const void* data, size_t dataLengthBytes, unsigned int dataLengthBits);

			/**
			 * \brief Revert line of "data" buffer in "revertedData" with new size (revertedDataLength)
			 * \param revertedData Buffer to be written
			 * \param revertedDataLengthBytes Length of "revertedData" in bytes
			 * \param data Buffer to be readed
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 * \return Length of reverted data (in bits)
			 */
			static unsigned int revert(void* revertedData, size_t revertedDataLengthBytes,
								 const void* data, size_t dataLengthBytes, unsigned int dataLengthBits);

			/**
			 * \brief Truncate "data" buffer in "truncatedData" in Little Endian
			 * \param truncatedData Buffer to be written
			 * \param truncatedDataLengthBytes Length of "truncatedData" in bytes
			 * \param data Buffer to be readed
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 * \return Length of truncated data (in bits)
			 */
			static unsigned int truncateLittleEndian(void* truncatedData, size_t truncatedDataLengthBytes,
											   const void* data, size_t dataLengthBytes, unsigned int dataLengthBits);

			/**
			 * \brief Swap all BYTES of "data" buffer in "swapedData"
			 * \param swapedData Buffer to be written
			 * \param swapedDataLengthBytes Length of "swapedData" in bytes
			 * \param data Buffer to be readed
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 */
			static void swapBytes(void* swapedData, size_t swapedDataLengthBytes,
								  const void* data, size_t dataLengthBytes, unsigned int dataLengthBits);

			/**
			 * \brief Extract "readPosBits" bits of data
			 * \param extractData Buffer to be written
			 * \param extractDataLengthBytes Length of "extractData" in bytes
			 * \param data Buffer to be readed
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 * \param readPosBits Offset of "data" you want to start to extract (in bits)
			 * \param readLengthBits Length to extract from "data" starting at "readPosBits" offset (in bits)
			 * \return Length of data written (in bits)
			 */
			static unsigned int extract(void* extractData, size_t extractDataLengthBytes,
								  const void* data, size_t dataLengthBytes, unsigned int dataLengthBits,
								  unsigned int readPosBits, unsigned int readLengthBits);

			/**
			 * \brief Write "data" buffer into "data" buffer
			 * \param writtenData Buffer to be modified
			 * \param writtenDataLengthBytes Length of data (in bytes)
			 * \param writePosBits Offset in "writtenData" buffer you want to start to write (in bits)
			 * \param data to be written in "writtenData"
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 */
			static void writeToBit(void* writtenData, size_t writtenDataLengthBytes,
								   unsigned int* writePosBits,
								   const void* data, size_t dataLengthBytes, unsigned int dataLengthBits);

			/**
			 * \brief Write "data" buffer into "writtenData" buffer
			 * \param writtenData Buffer to be modified
			 * \param writtenDataLengthBytes Length of data (in bytes)
			 * \param writePosBits Offset in "writtenData" buffer you want to start to write (in bits)
			 * \param data to be written in "writenData"
			 * \param dataLengthBytes Length of data in bytes
			 * \param dataLengthBits Length of used data in bits
			 * \param readPosBits Offset in "data" buffer you want to start to copy (in bits)
			 * \param readLengthBits Length of "data" buffer to write (in bits)
			 */
			static void writeToBit(void* writtenData, size_t writtenDataLengthBytes,
								   unsigned int* writePosBits,
								   const void* data, size_t dataLengthBytes, unsigned int dataLengthBits,
								   unsigned int readPosBits, unsigned int readLengthBits);

			/**
			 * \brief Write "data" buffer into "writtenData" buffer
			 * \param writtenData Buffer to be modified
			 * \param writtenDataLengthBytes Length of data (in bits)
			 * \param writePosBits Offset in "writtenData" buffer you want to start to write (in bits)
			 * \param data Byte to be written in "writtenData"
			 */
			static void writeToBit(void* writtenData, size_t writtenDataLengthBytes,
								   unsigned int* writePosBits,
								   unsigned char data);

			/**
			 * \brief Write "data" buffer into "writtenData" buffer
			 * \param writtenData Buffer to be modified
			 * \param writtenDataLengthBytes Length of data (in bits)
			 * \param writePosBits Offset in "writtenData" buffer you want to start to write (in bits)
			 * \param data Byte to be written in "writtenData"
			 * \param readPosBits Offset in "data" buffer you want to start to copy (in bits)
			 * \param readLengthBits Length of "data" buffer to write (in bits)
			 */
			static void writeToBit(void* writtenData, size_t writtenDataLengthBytes,
								   unsigned int* writePosBits,
								   unsigned char data,
								   unsigned int readPosBits, unsigned int readLengthBits);
	};
}

#endif /* LOGICALACCESS_BITHELPER_HPP */
