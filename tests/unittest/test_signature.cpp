#include <gtest/gtest.h>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/plugins/crypto/signature_helper.hpp"

using namespace logicalaccess;

static std::string pubkey = "-----BEGIN PUBLIC KEY-----\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwZ1H25sOi39JYwAlV0Tz\n\
d/Srw+ecCfdzSjYT5TjlLAPLoIVJYNS1zQeqfBYf1l5MdnwZcKz+rHSfKdljgcZF\n\
tIvJ+5+B0gf2YJY97eCY3BQocmuhnIKD8+msrQudMlSKE8k4pzaRWbZJe1PD053y\n\
2KoyiZXAkIxxEHxW84cLHvonyQn8MqswBrLVW8g77Ndui0rvLHVRb0BMy7smFSGI\n\
D9tIvRkHPm6vrKo2gq2dvQfH6ApttFObYi75kuOxNaS+xUbipB+TMzklhT35z7dC\n\
El75fICdJ5f9ZY58Mpf4UJ6LPu8tBkTFekeZiKHUE3kAkVLukcSBpDTTmvcUX1CZ\n\
jQIDAQAB\n\
-----END PUBLIC KEY-----\n";

TEST(test_signature, verify1)
{
    auto blob = BufferHelper::getStdString(
        BufferHelper::fromHexString("0A10000010920A438C66000000000000000010021A10D43DEBC7"
                                    "D61B4127A01EC5B1EA3923C820BCA1A083A92C"));
    auto signature = BufferHelper::getStdString(BufferHelper::fromHexString(
        "4D9C69814ED33793E2FB5528AEFD651A3F933C76F4943ECE3DFD62D3A471530BFA5753EF46760F13"
        "9E560095B2E3C4FB0486A1D8686473F69C4F4911BDBCB833A75FBF5782E240B16A91FA64A152F2E7"
        "E6E72EBBC2AABF1CFC3887CD9583018272C6F600E25E0D87010C0260EACAB7C159403DA88725AF94"
        "E3503E5D97A98E7F01EFC9C10E0BEA5E52E85812F5B8BB4814D85A76EAD916391ED3E198CA00AA57"
        "34588178F4F922AC2DB354D067A7331C89ECC1F3189E689314DFD6302DA37D5803D2ACF9A0D54D70"
        "36BBC3E055B6687B95349F1DFE2C2C859FD32AC2DF33CBD2B812DAA668EA43E4F9B103E0E9A0D54A"
        "D78F1EB682F83C6744F021FD0907B19F"));
    ASSERT_TRUE(SignatureHelper::verify_sha512(blob, signature, pubkey));

    auto invalid_signature  = signature;
    invalid_signature.at(0) = '3';
    ASSERT_FALSE(SignatureHelper::verify_sha512(blob, invalid_signature, pubkey));
}
