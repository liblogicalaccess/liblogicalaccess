#include "logicalaccess/utils.hpp"
#include <boost/asio.hpp>
#include <logicalaccess/logs.hpp>
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
}