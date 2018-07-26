#include <gtest/gtest.h>
#include <logicalaccess/bufferhelper.hpp>
#include "logicalaccess/TLV.hpp"


TEST(test_tlv, parse)
{
    using namespace logicalaccess;

    // We have 3 TLV here.
    std::string hexstr = "80050680800A4081102B0601040181E43801010204018F6313820107";

    size_t consumed;
    auto tlvs = parse_tlvs(BufferHelper::fromHexString(hexstr), consumed);
    ASSERT_EQ(3, tlvs.size());
    // Only the last 2 byttes, 9000 should have not been consumed.
    ASSERT_EQ(BufferHelper::fromHexString(hexstr).size(), consumed);

    // Assert TLV 0
    ASSERT_EQ(0x80, tlvs[0]->tag());
    ASSERT_EQ(5, tlvs[0]->value().size());

    ASSERT_EQ(0x81, tlvs[1]->tag());
    ASSERT_EQ(0x10, tlvs[1]->value().size());

    ASSERT_EQ(0x82, tlvs[2]->tag());
    ASSERT_EQ(1, tlvs[2]->value().size());
}
