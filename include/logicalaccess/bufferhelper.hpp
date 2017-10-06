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

namespace logicalaccess
{
    /**
     * \brief A buffer helper class.
     */
    class LIBLOGICALACCESS_API BufferHelper
    {
    public:

        static std::string toBase64(const std::vector<unsigned char>& buf);

        static std::vector<unsigned char> fromBase64(const std::string& b64str);

        static std::string getHex(const std::vector<unsigned char>& buffer);

        static std::vector<unsigned char> fromHexString(std::string hexString);

        static std::string getStdString(const std::vector<unsigned char>& buffer);

        static void setUShort(std::vector<unsigned char>& buffer, const unsigned short& value);

        static unsigned short getUShort(const std::vector<unsigned char>& buffer, size_t& offset);

        static void setInt32(std::vector<unsigned char>& buffer, const long& value);

        static long getInt32(const std::vector<unsigned char>& buffer, size_t& offset);

        static unsigned long getUInt32(const std::vector<unsigned char>& buffer, size_t& offset);

        static void setUInt32(std::vector<unsigned char>& buffer, const unsigned long& value);

        static void setUInt64(std::vector<unsigned char>& buffer, const unsigned long long& value);

        static uint64_t getUInt64(const std::vector<unsigned char>& buffer, size_t& offset);

		static void setUInt16(std::vector<unsigned char>& buffer, const unsigned long& value);

		static unsigned long getUInt16(const std::vector<unsigned char>& buffer, size_t& offset);

        static void setString(std::vector<unsigned char>& buffer, const std::string& value);

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