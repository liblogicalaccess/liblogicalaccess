/**
 * \file bufferhelper.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Buffer Helper class.
 */

#include "logicalaccess/bufferhelper.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
/**
    * \brief Encode a chunk of data into base 64.
    * \param in The chunk of data to encode.
    * \param out The buffer to fill with the encoded data.
    * \param len The length of in (must be 3 bytes max).
    */
static void base64EncodeBlock(const unsigned char in[3], unsigned char out[4], size_t len)
{
    static const char cb64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    out[0] = cb64[in[0] >> 2];
    out[1] = cb64[((in[0] & 0x03) << 4) | (len > 1 ? ((in[1] & 0xf0) >> 4) : 0)];
    out[2] =
        (len > 1 ? cb64[((in[1] & 0x0f) << 2) | (len > 2 ? ((in[2] & 0xc0) >> 6) : 0)]
                 : '=');
    out[3] = (len > 2 ? cb64[in[2] & 0x3f] : '=');
}

/**
    * \brief Decode a chunk of base 64 encoded data.
    * \param in The encoded chunk of data.
    * \param out The out buffer.
    * \param len The length of the out buffer.
    */
static void base64DecodeBlock(const unsigned char in[4], unsigned char out[3],
                              size_t &len)
{
    len = 3;

    for (size_t i = 0; i < 4; ++i)
    {
        EXCEPTION_ASSERT(
            isalnum(in[i]) || (in[i] == '+') || (in[i] == '/') ||
                ((i >= 2) && (in[i] == '=')),
            LibLogicalAccessException,
            (std::string("Unexpected character '") + static_cast<char>(in[i]) + "'")
                .c_str());
    }

    if (in[2] == '=')
    {
        if (in[3] == '=')
        {
            len = 1;
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "'=' character expected");
        }
    }
    else
    {
        if (in[3] == '=')
        {
            len = 2;
        }
    }

    static char tin[4] = {0x00, 0x00, 0x00, 0x00};

    memcpy(tin, in, 4);

    for (size_t i = 0; i < 4; ++i)
    {
        if ((tin[i] >= 'A') && (tin[i] <= 'Z'))
        {
            tin[i] -= 'A';
        }
        else
        {
            if ((tin[i] >= 'a') && (tin[i] <= 'z'))
            {
                tin[i] -= 'a' - ('Z' - 'A' + 1);
            }
            else
            {
                if ((tin[i] >= '0') && (tin[i] <= '9'))
                {
                    tin[i] -= '0' - ('Z' - 'A' + 1) - ('z' - 'a' + 1);
                }
                else
                {
                    if (tin[i] == '+')
                    {
                        tin[i] = 62;
                    }
                    else
                    {
                        if (tin[i] == '/')
                        {
                            tin[i] = 63;
                        }
                        else
                        {
                            tin[i] = 0;
                        }
                    }
                }
            }
        }
    }

    out[0] = (tin[0] << 2 | tin[1] >> 4);
    out[1] = (tin[1] << 4 | tin[2] >> 2);
    out[2] = (((tin[2] << 6) & 0xc0) | tin[3]);
}

std::string BufferHelper::getHex(const ByteVector &buffer)
{
    std::ostringstream ss;

    ss << std::hex << std::uppercase << std::setfill('0');
    std::for_each(buffer.cbegin(), buffer.cend(),
                  [&](int c) { ss << std::setw(2) << c; });

    std::string result = ss.str();
    return result;
}

std::string BufferHelper::toBase64(const ByteVector &buf)
{
    std::string result;
    result.resize(((buf.size() + 2) / 3) * 4);

    size_t result_len = 0;

    for (size_t i = 0; i < buf.size();)
    {
        size_t len = (buf.size() - i);

        if (len > 3)
        {
            len = 3;
        }

        result_len += 4;
        base64EncodeBlock(
            &buf[i], reinterpret_cast<unsigned char *>(&result[0] + result_len - 4), len);

        i += len;
    }

    return result;
}

ByteVector BufferHelper::fromBase64(const std::string &b64str)
{
    EXCEPTION_ASSERT((b64str.size() % 4) == 0, std::invalid_argument,
                     "The buffer size must be multiple of 4");

    ByteVector result;
    unsigned char tmp[64];

    for (size_t i = 0; i < b64str.size(); i += 4)
    {
        size_t len = 0;
        base64DecodeBlock(reinterpret_cast<const unsigned char *>(&b64str[0] + i), tmp,
                          len);

        result.insert(result.end(), tmp, tmp + len);

        if (len < 3)
        {
            break;
        }
    }

    return result;
}

ByteVector BufferHelper::fromHexString(std::string hexString)
{
    ByteVector data;
    std::stringstream convertStream;

    // delete spaces
    hexString.erase(std::remove(hexString.begin(), hexString.end(), ' '),
                    hexString.end());

    size_t offset = 0;
    while (offset < hexString.length())
    {
        unsigned int buffer;

        convertStream << std::hex << hexString.substr(offset, 2);
        convertStream >> std::hex >> buffer;

        data.push_back(static_cast<unsigned char>(buffer));

        offset += 2;

        // empty the stringstream
        convertStream.str(std::string());
        convertStream.clear();
    }

    return data;
}

std::string BufferHelper::getStdString(const ByteVector &buffer)
{
    return std::string(buffer.begin(), buffer.end());
}

void BufferHelper::setUShort(ByteVector &buffer, const unsigned short &value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xff));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00) >> 8));
}

unsigned short BufferHelper::getUShort(const ByteVector &buffer, size_t &offset)
{
    unsigned short tmp = static_cast<unsigned short>(buffer[offset++]);
    tmp |= static_cast<unsigned short>(buffer[offset++]) << 8;
    return tmp;
}

void BufferHelper::setInt32(ByteVector &buffer, const long &value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xff));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00) >> 8));
    buffer.push_back(static_cast<unsigned char>((value & 0xff0000) >> 16));
    buffer.push_back(static_cast<unsigned char>((value & 0xff000000) >> 24));
}

void BufferHelper::setUInt16(ByteVector &buffer, const unsigned long &value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xff));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00) >> 8));
}

void BufferHelper::setUInt32(ByteVector &buffer, const unsigned long &value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xff));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00) >> 8));
    buffer.push_back(static_cast<unsigned char>((value & 0xff0000) >> 16));
    buffer.push_back(static_cast<unsigned char>((value & 0xff000000) >> 24));
}

unsigned long BufferHelper::getUInt16(const ByteVector &buffer, size_t &offset)
{
    unsigned long tmp = static_cast<unsigned long>(buffer[offset++]);
    tmp |= static_cast<unsigned long>(buffer[offset++]) << 8;
    return tmp;
}

unsigned long BufferHelper::getUInt32(const ByteVector &buffer, size_t &offset)
{
    unsigned long tmp = static_cast<unsigned long>(buffer[offset++]);
    tmp |= static_cast<unsigned long>(buffer[offset++]) << 8;
    tmp |= static_cast<unsigned long>(buffer[offset++]) << 16;
    tmp |= static_cast<unsigned long>(buffer[offset++]) << 24;
    return tmp;
}

long BufferHelper::getInt32(const ByteVector &buffer, size_t &offset)
{
    long tmp = static_cast<long>(buffer[offset++]);
    tmp |= static_cast<long>(buffer[offset++]) << 8;
    tmp |= static_cast<long>(buffer[offset++]) << 16;
    tmp |= static_cast<long>(buffer[offset++]) << 24;
    return tmp;
}

void BufferHelper::setUInt64(ByteVector &buffer, const unsigned long long &value)
{
    buffer.push_back(static_cast<unsigned char>(value & 0xff));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00) >> 8));
    buffer.push_back(static_cast<unsigned char>((value & 0xff0000) >> 16));
    buffer.push_back(static_cast<unsigned char>((value & 0xff000000) >> 24));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00000000) >> 32));
    buffer.push_back(static_cast<unsigned char>((value & 0xff0000000000) >> 40));
    buffer.push_back(static_cast<unsigned char>((value & 0xff000000000000) >> 48));
    buffer.push_back(static_cast<unsigned char>((value & 0xff00000000000000) >> 56));
}

uint64_t BufferHelper::getUInt64(const ByteVector &buffer, size_t &offset)
{
    uint64_t tmp = static_cast<uint64_t>(buffer[offset++]);
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 8;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 16;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 24;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 32;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 40;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 48;
    tmp |= static_cast<uint64_t>(buffer[offset++]) << 56;
    return tmp;
}

void BufferHelper::setString(ByteVector &buffer, const std::string &value)
{
    buffer.insert(buffer.end(), value.begin(), value.end());
}
}