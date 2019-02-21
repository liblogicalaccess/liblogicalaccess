#include <gtest/gtest.h>
#include <regex>

TEST(test_regex, match_simple)
{
    std::regex reg("Simple Test \\(.*\\) EndOfString");
    std::string real_name = "Simple Test (SomeContent) EndOfString";

    ASSERT_TRUE(std::regex_match(real_name, reg));

    real_name = "Simple Test (SomeContent) DontMatch";
    ASSERT_FALSE(std::regex_match(real_name, reg));
}

TEST(test_regex, match_reader_name)
{
    std::regex reg(
        R"(HID Global OMNIKEY 5422 Smartcard Reader \[OMNIKEY 5422CL Smartcard Reader\] \(.*\) 00 00)");
    std::string real_name = "HID Global OMNIKEY 5422 Smartcard Reader [OMNIKEY 5422CL Smartcard Reader] (KJ0H6V004W10673746) 00 00";

    ASSERT_TRUE(std::regex_match(real_name, reg));
}

TEST(test_regex, fail_regex)
{
    auto create_invalid_regex = []() {
        std::regex reg(R"((.)");
    };
    ASSERT_THROW(create_invalid_regex(),
                 std::regex_error);
}
