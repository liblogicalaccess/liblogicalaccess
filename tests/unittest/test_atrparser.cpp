#include "logicalaccess/lla_fwd.hpp"
#include "pluginsreaderproviders/pcsc/atrparser.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace logicalaccess;

TEST(test_atr_parser, test_1)
{
    ASSERT_EQ("DESFire",
              ATRParser::guessCardType(ByteVector{0x3b, 0x81, 0x80, 0x1, 0x80, 0x80}));

    ASSERT_EQ("Mifare1K", ATRParser::guessCardType(ByteVector{
                              0x3b, 0x8f, 0x80, 0x1, 0x80, 0x4f, 0xc, 0xa0, 0, 0,
                              3,    6,    3,    0,   1,    0,    0,   0,    0, 0x6a}));

    ASSERT_EQ("HIDiClass16KS", ATRParser::guessCardType(ByteVector{
                                   0x3b, 0x8f, 0x80, 1, 0x80, 0x4f, 0xc, 0xa0, 0, 0,
                                   3,    6,    0xa,  0, 0x1a, 0,    0,   0,    0, 0x78}));
    ASSERT_EQ("HIDiClass16KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A001A0000000078"));

    ASSERT_EQ("HIDiClass2KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A0018000000007A"));

    ASSERT_EQ("ISO15693",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060B00140000000077"));
    ASSERT_EQ("HIDiClass8x2KS",
              ATRParser::guessCardType("3B8F8001804F0CA0000003060A001C000000007E"));
}

TEST(test_atr_parser, test_hardcoded)
{
    ASSERT_EQ("UNKNOWN",
              ATRParser::guessCardType("3B8F8001804F0CA0000003064000000000000028"));
    ASSERT_EQ("Prox", ATRParser::guessCardType("3B8F8001804F0CA0000003064000000000000028",
                                               PCSC_RUT_OMNIKEY_XX27));

    ASSERT_EQ("MifarePlus_SL1_2K",
              ATRParser::guessCardType("3B8F8001804F0CA000000306030001000000006A",
                                       PCSC_RUT_ACS_ACR_1222L));
}
