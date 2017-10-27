#include "logicalaccess/lla_fwd.hpp"
#include <bitset>
#include <gtest/gtest.h>
#include <logicalaccess/utils.hpp>


using namespace logicalaccess;

TEST(test_manchester, encode_ieee)
{
    ByteVector in       = {0x01, 0x0};
    ByteVector expected = {0xAA, 0xA9, 0xAA, 0xAA};
    auto out            = ManchesterEncoder::encode(in, ManchesterEncoder::IEEE_802);
    ASSERT_EQ(expected, out);
}

TEST(test_manchester, encode_bits_ieee)
{
    std::bitset<16> in;
    in[7] = 1;

    ByteVector expected = {0xAA, 0xA9, 0xAA, 0xAA};
    auto out            = ManchesterEncoder::encode(in, ManchesterEncoder::IEEE_802);
    auto out_vector     = bitsetToVector(out);
    ASSERT_EQ(expected, out_vector);
}

TEST(test_manchester, encode_thomas)
{
    ByteVector in       = {0x01, 0x0};
    ByteVector expected = {0x55, 0x56, 0x55, 0x55};
    auto out            = ManchesterEncoder::encode(in, ManchesterEncoder::G_E_THOMAS);
    ASSERT_EQ(expected, out);
}

TEST(test_manchester, decode_ieee)
{
    ByteVector in       = {0xAA, 0xA9, 0xAA, 0xAA};
    ByteVector expected = {0x01, 0x0};
    auto out            = ManchesterEncoder::decode(in, ManchesterEncoder::IEEE_802);
    ASSERT_EQ(expected, out);
}

TEST(test_manchester, decode_thomas)
{
    ByteVector in       = {0x55, 0x56, 0x55, 0x55};
    ByteVector expected = {0x01, 0x0};
    auto out            = ManchesterEncoder::decode(in, ManchesterEncoder::G_E_THOMAS);
    ASSERT_EQ(expected, out);
}
