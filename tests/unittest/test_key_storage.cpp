#include <gtest/gtest.h>
#include <logicalaccess/utils.hpp>
#include <logicalaccess/cards/computermemorykeystorage.hpp>

using namespace logicalaccess;

TEST(test_key_storage, test_serialize)
{
    ComputerMemoryKeyStorage s;

    s.addMetadata("Lama", "Sticot");
    s.addMetadata("Lama2", "Sticot2");
    s.addMetadata("Lama3", "Sticot3");

    ASSERT_TRUE(s.hasMetadata("Lama"));
    ASSERT_TRUE(s.hasMetadata("Lama3"));
    ASSERT_TRUE(s.hasMetadata("Lama2"));

    ASSERT_EQ("Sticot", s.getMetadata("Lama"));
    ASSERT_EQ("Sticot2", s.getMetadata("Lama2"));
    ASSERT_EQ("Sticot3", s.getMetadata("Lama3"));

    std::cout << s.serialize() << std::endl;
}

TEST(test_key_storage, test_unserialize)
{
    ComputerMemoryKeyStorage s;
    std::string data = "<ComputerMemoryKeyStorage><KeyStorage><metadata><Lama>Sticot</"
                       "Lama><Lama2>Sticot2</Lama2><Lama3>Sticot3</Lama3></metadata></"
                       "KeyStorage></ComputerMemoryKeyStorage>";

    s.unSerialize(data, "");

    ASSERT_TRUE(s.hasMetadata("Lama"));
    ASSERT_TRUE(s.hasMetadata("Lama3"));
    ASSERT_TRUE(s.hasMetadata("Lama2"));
    ASSERT_EQ("Sticot", s.getMetadata("Lama"));
    ASSERT_EQ("Sticot2", s.getMetadata("Lama2"));
    ASSERT_EQ("Sticot3", s.getMetadata("Lama3"));
}
