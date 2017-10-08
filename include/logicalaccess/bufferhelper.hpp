/**
 * \file bufferhelper.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Buffer Helper class.
 */

#ifndef BUFFERHELPER_HPP
#define BUFFERHELPER_HPP

#include <stdint.h>
#include <vector>
#include <string>
#include "logicalaccess/logicalaccess_api.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"

namespace logicalaccess
{
    /**
     * \brief A buffer helper class.
     */
    class LIBLOGICALACCESS_API BufferHelper
    {
    public:

        static std::string toBase64(const ByteVector& buf);

        static ByteVector fromBase64(const std::string& b64str);

        static std::string getHex(const ByteVector& buffer);

        static ByteVector fromHexString(std::string hexString);

        static std::string getStdString(const ByteVector& buffer);

        static void setUShort(ByteVector& buffer, const unsigned short& value);

        static unsigned short getUShort(const ByteVector& buffer, size_t& offset);

        static void setInt32(ByteVector& buffer, const long& value);

        static long getInt32(const ByteVector& buffer, size_t& offset);

        static unsigned long getUInt32(const ByteVector& buffer, size_t& offset);

        static void setUInt32(ByteVector& buffer, const unsigned long& value);

        static void setUInt64(ByteVector& buffer, const unsigned long long& value);

        static uint64_t getUInt64(const ByteVector& buffer, size_t& offset);

		static void setUInt16(ByteVector& buffer, const unsigned long& value);

		static unsigned long getUInt16(const ByteVector& buffer, size_t& offset);

        static void setString(ByteVector& buffer, const std::string& value);

        /**
         * Check if the buffer contains either no byte, or only zeroed bytes
         */
        template<typename Buffer>
        static bool allZeroes(const Buffer &buf)
        {
            for (const auto & byte : buf)
            {
                if (byte)
                    return false;
            }
            return true;
        }
    };
}

#endif /* BUFFERHELPER_HPP */