/**
 * \file bithelper.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Bit helper.
 */

#ifndef LOGICALACCESS_BITHELPER_HPP
#define LOGICALACCESS_BITHELPER_HPP

#include "logicalaccess/services/accesscontrol//formats/BitsetStream.hpp"
#include "logicalaccess/readerproviders/readerprovider.hpp"

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
		static BitsetStream align(
			const BitsetStream& data, unsigned int dataLengthBits);


        /**
         * \brief Revert line of "data" buffer in "revertedData" with new size (revertedDataLength)
         * \param revertedData Buffer to be written
         * \param revertedDataLengthBytes Length of "revertedData" in bytes
         * \param data Buffer to be readed
         * \param dataLengthBytes Length of data in bytes
         * \param dataLengthBits Length of used data in bits
         * \return Length of reverted data (in bits)
         */
        static BitsetStream revert(const BitsetStream& data, unsigned int dataLengthBits);

        /**
         * \brief Truncate "data" buffer in "truncatedData" in Little Endian
         * \param truncatedData Buffer to be written
         * \param truncatedDataLengthBytes Length of "truncatedData" in bytes
         * \param data Buffer to be readed
         * \param dataLengthBytes Length of data in bytes
         * \param dataLengthBits Length of used data in bits
         * \return Length of truncated data (in bits)
         */
        static BitsetStream truncateLittleEndian(
            const BitsetStream& data, unsigned int dataLengthBits);

        /**
         * \brief Swap all BYTES of "data" buffer in "swapedData"
         * \param swapedData Buffer to be written
         * \param swapedDataLengthBytes Length of "swapedData" in bytes
         * \param data Buffer to be readed
         * \param dataLengthBytes Length of data in bytes
         * \param dataLengthBits Length of used data in bits
         */
        static BitsetStream swapBytes(const BitsetStream& data);

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
        static BitsetStream extract(const BitsetStream& data, unsigned int readPosBits,
			unsigned int readLengthBits);
    };
}

#endif /* LOGICALACCESS_BITHELPER_HPP */