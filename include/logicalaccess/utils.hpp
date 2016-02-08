#pragma once

#include "lla_fwd.hpp"
#include "logicalaccess_api.hpp"
#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>

/**
 * Platform independent utils.
 */

namespace logicalaccess
{
uint32_t lla_htonl(uint32_t in);
uint16_t lla_htons(uint16_t in);

/**
 * This class provide a simple to get the elapsed time since
 * it's creation.
 *
 * The precision of the counter is milliseconds.
 */
class LIBLOGICALACCESS_API ElapsedTimeCounter
{
  public:
    ElapsedTimeCounter();

    /**
     * Returns the elapsed time since the creation of
     * the ElapsedTimeCounter object.
     */
    size_t elapsed() const;

  private:
    using TimePoint = std::chrono::steady_clock::time_point;

    TimePoint creation_;
};

template <typename T>
struct GetBitSetSize;

template <size_t Len>
struct GetBitSetSize<std::bitset<Len>>
{
    enum
    {
        Length = Len
    };
};

/**
 * Convert a bitset to a byte vector.
 * If the bitset's length is not modulo 8, compile time error
 */
template <typename BitSet>
ByteVector bitsetToVector(const BitSet &in)
{
    static_assert(GetBitSetSize<BitSet>::Length % 8 == 0,
                  "Cannot convert a bitset "
                  "whose length is not modulo 8");
    const size_t len = GetBitSetSize<BitSet>::Length / 8;
    ByteVector out(len);

    size_t count = 0;
    for (size_t i = 0; i < len; ++i)
    {
        uint8_t b = 0;
        for (int j = 0; j < 8; ++j)
        {
            b |= in[count] << (7 - j);
            count++;
        }
        out[i] = b;
    }
    return out;
}

/**
 * An helper class to perform Manchester encoding/decoding.
 *
 * See http://eleif.net/manchester.html for an online encoder.
 */
class ManchesterEncoder
{
  public:
    enum Type
    {
        G_E_THOMAS,
        IEEE_802
    };

    /**
     * Encode a bitset, returns a bitset twice as large.
     */
    template <typename BitSet>
    static std::bitset<GetBitSetSize<BitSet>::Length * 2> encode(const BitSet &in,
                                                                 Type t)
    {
        std::bitset<GetBitSetSize<BitSet>::Length * 2> out;

        for (int count = 0, read_count = 0;
             count < GetBitSetSize<BitSet>::Length * 2; read_count++)
        {
            if ((in[read_count] && t == IEEE_802) ||
                (!in[read_count] && t == G_E_THOMAS))
            {
                out[count++] = 0;
                out[count++] = 1;
            }
            else
            {
                out[count++] = 1;
                out[count++] = 0;
            }
        }
        return out;
    }

    static ByteVector encode(const ByteVector &in, Type t);
    static ByteVector decode(const ByteVector &in, Type t);
};
}
