#include <gtest/gtest.h>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/utils.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgutils.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/tlv.hpp>

using namespace logicalaccess;

TEST(test_stid_prg_util, build_prox_encoding_w26)
{
    ByteVector bytes = {0x00, 0x10, 0x70, 0x60, 0x1D, 0x55, 0x59, 0x55,
                        0x55, 0x69, 0xA5, 0x69, 0x5A, 0x95, 0x65, 0x99};

    Wiegand26Format fmt;
    fmt.setUid(0x1C25);
    fmt.setFacilityCode(0x63);

    auto ret = STidPRGUtils::prox_configuration_bytes(fmt);
    std::cout << std::endl;

    ASSERT_EQ(bytes, ret);
}

TEST(test_stid_prg_util, build_prox_encoding_w26_2)
{
    ByteVector bytes = {0x00, 0x10, 0x70, 0x60, 0x1D, 0x55, 0x59, 0x55,
                        0x55, 0x65, 0x66, 0x65, 0x96, 0x96, 0x59, 0x6A};
    Wiegand26Format fmt;
    fmt.setUid(0x4C93);
    fmt.setFacilityCode(0x2A);

    auto ret = STidPRGUtils::prox_configuration_bytes(fmt);
    ASSERT_EQ(bytes, ret);
}

TEST(test_stid_prg_util, build_prox_encoding_fw37)
{
    ByteVector bytes = {0x00, 0x10, 0x70, 0x60, 0x1D, 0x55, 0x55, 0x55,
                        0x56, 0x66, 0xA9, 0xAA, 0xA6, 0xA5, 0xAA, 0x6A};

    Wiegand37WithFacilityFormat fmt;
    fmt.setUid(519803);
    fmt.setFacilityCode(350);

    auto ret = STidPRGUtils::prox_configuration_bytes(fmt);
    std::cout << std::endl;

    ASSERT_EQ(bytes, ret);
}
