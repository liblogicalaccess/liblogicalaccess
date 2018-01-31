#include <gtest/gtest.h>
#include <logicalaccess/cards/aes128key.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/plugins/crypto/signature_helper.hpp"

using namespace logicalaccess;

TEST(test_diversificaiton, av2)
{
    auto k = std::make_shared<DESFireKey>();
    k->setKeyType(DESFireKeyType::DF_KEY_AES);
    k->fromString("00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF");

    auto div = std::make_shared<NXPAV2KeyDiversification>();
    k->setKeyDiversification(div);
    auto user_div_input =
        BufferHelper::fromHexString("04782E21801D803042F54E585020416275");

    auto diversified = div->getDiversifiedKey(k, user_div_input);

    ASSERT_EQ(BufferHelper::fromHexString("A8DD63A3B89D54B37CA802473FDA9175"),
              diversified);
}

TEST(test_diversificaiton, av2_2)
{
    auto k = std::make_shared<DESFireKey>();
    k->setKeyType(DESFireKeyType::DF_KEY_AES);
    k->fromString("f3 f9 37 76 98 70 7b 68 8e af 84 ab e3 9e 37 91");

    auto div = std::make_shared<NXPAV2KeyDiversification>();
    k->setKeyDiversification(div);
    auto user_div_input = BufferHelper::fromHexString("04deadbeeffeed");

    auto diversified = div->getDiversifiedKey(k, user_div_input);

    ASSERT_EQ(BufferHelper::fromHexString("0bb408baff98b6ee9f2e1585777f6a51"),
              diversified);
}
