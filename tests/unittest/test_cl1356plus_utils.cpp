#include <gtest/gtest.h>
#include <logicalaccess/utils.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/CL1356PlusUtils.hpp>

using namespace logicalaccess;

TEST(test_cl1356plus_util, get_card_info)
{
    ByteVector bytes = {0x01, 0x04, 0x00, 0x00, 0x01, 0x07, 0x04, 0x60, 0x55, 0x19,
                        0x06, 0x1F, 0x80, 00,   00,   00,   0x20, 0x75, 0x77, 0x81,
                        0x02, 0x01, 0x80, 00,   00,   00,   00,   00,   00,   00,
                        00,   00,   00,   00,   00,   00,   00};
    auto info = CL1356PlusUtils::parse_get_card_info(bytes);

    auto expected = ByteVector{0x04, 0x60, 0x55, 0x19, 0x06, 0x1F, 0x80};
    ASSERT_EQ(expected, info.uid_);
    ASSERT_EQ(CL1356PlusUtils::Info::State::HALT, info.state_);
}
