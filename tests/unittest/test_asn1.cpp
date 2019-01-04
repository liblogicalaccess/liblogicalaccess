#include <gtest/gtest.h>
#include "logicalaccess/asn1.hpp"


TEST(test_asn1, encode_oid)
{
    using namespace logicalaccess;

	ASSERT_EQ(
        ASN1::encode_oid(std::vector<unsigned int>({1, 3, 6, 1, 4, 1, 311, 21, 20})),
        ByteVector({0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x15, 0x14}));

    ASSERT_EQ(ASN1::encode_oid("1.3.6.1.4.1.311.21.20"),
                  ByteVector({0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x15, 0x14}));
}
