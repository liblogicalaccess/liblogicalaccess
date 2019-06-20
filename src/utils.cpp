#include <logicalaccess/utils.hpp>
#include <boost/asio.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

using namespace std::chrono;

namespace logicalaccess
{
uint32_t lla_htonl(uint32_t in)
{
    return htonl(in);
}

uint16_t lla_htons(uint16_t in)
{
    return htons(in);
}

ElapsedTimeCounter::ElapsedTimeCounter()
{
    creation_ = steady_clock::now();
}

size_t ElapsedTimeCounter::elapsed() const
{
    auto diff = steady_clock::now() - creation_;
    return static_cast<size_t>(duration_cast<milliseconds>(diff).count());
}

size_t ElapsedTimeCounter::elapsed_micro() const
{
    auto diff = steady_clock::now() - creation_;
    return static_cast<size_t>(duration_cast<microseconds>(diff).count());
}

ByteVector ManchesterEncoder::encode(const ByteVector &in, Type t)
{
    ByteVector out;
    for (const auto &byte : in)
    {
        uint8_t b0 = 0;
        for (int i = 0; i < 4; ++i)
        {
            bool bit = (byte >> i) & 0x01;
            if (bit)
            {
                if (t == IEEE_802)
                    b0 |= 1 << (i * 2);
                else
                    b0 |= 1 << (i * 2 + 1);
            }
            else
            {
                if (t == IEEE_802)
                    b0 |= 1 << (i * 2 + 1);
                else
                    b0 |= 1 << (i * 2);
            }
        }

        uint8_t b1 = 0;
        for (int i = 0; i < 4; ++i)
        {
            bool bit = (byte >> (4 + i)) & 0x01;
            if (bit)
            {
                if (t == IEEE_802)
                    b1 |= 1 << (i * 2);
                else
                    b1 |= 1 << (i * 2 + 1);
            }
            else
            {
                if (t == IEEE_802)
                    b1 |= 1 << (i * 2 + 1);
                else
                    b1 |= 1 << (i * 2);
            }
        }
        out.push_back(b1);
        out.push_back(b0);
    }
    return out;
}

ByteVector ManchesterEncoder::decode(const ByteVector &in, Type t)
{
    ByteVector out;
    EXCEPTION_ASSERT_WITH_LOG(in.size() % 2 == 0 && in.size(), LibLogicalAccessException,
                              "Input vector size shall be even and non zero");
    auto itr = in.begin();
    while (itr != in.end())
    {
        uint8_t offset = 0;
        uint8_t b      = 0;
        for (int i = 0; i < 8; i += 2)
        {
            uint8_t tmp = 0;
            tmp |= ((*itr >> i) & 0x01);
            tmp |= ((*itr >> (i + 1)) & 0x01) << 1;

            if (tmp == 1) // means 01
            {
                if (t == IEEE_802)
                    b |= 1 << offset;
                else
                    b |= 0 << offset; // NOOP
            }
            if (tmp == 2) // means 10
            {
                if (t == IEEE_802)
                    b |= 0 << offset; // NOOP
                else
                    b |= 1 << offset;
            }
            offset++;
        }

        // Now again for next byte
        ++itr;
        assert(itr != in.end());
        for (int i = 0; i < 8; i += 2)
        {
            uint8_t tmp = 0;
            tmp |= ((*itr >> i) & 0x01);
            tmp |= ((*itr >> (i + 1)) & 0x01) << 1;

            if (tmp == 1) // means 01
            {
                if (t == IEEE_802)
                    b |= 1 << offset;
                else
                    b |= 0 << offset; // NOOP
            }
            if (tmp == 2) // means 10
            {
                if (t == IEEE_802)
                    b |= 0 << offset; // NOOP
                else
                    b |= 1 << offset;
            }
            offset++;
        }
        ++itr;

        // We need to swap 4 bits in the byte.
        out.push_back(((b & 0x0F) << 4) | ((b & 0xF0) >> 4));
    }
    return out;
}

int portable_setenv(const char *name, const char *value, int overwrite)
{
#ifdef _WIN32
    int errcode = 0;
    if (!overwrite)
    {
        size_t envsize = 0;
        errcode        = getenv_s(&envsize, NULL, 0, name);
        if (errcode || envsize)
            return errcode;
    }
    return _putenv_s(name, value);
#else
    return setenv(name, value, overwrite);
#endif
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";


static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const *bytes_to_encode, unsigned int in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] =
                ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] =
                ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j              = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::string base64_decode(std::string const &encoded_string)
{
    size_t in_len = encoded_string.size();
    size_t i      = 0;
    size_t j      = 0;
    int in_       = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') &&
           is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                char_array_4[i] =
                    static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] =
                ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i)
    {
        for (j              = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] =
                static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}

}