#include <gtest/gtest.h>
#include <vector>
#include <stdint.h>
#include <climits>
#include <logicalaccess/services/accesscontrol/formats/BitsetStream.hpp>

using namespace logicalaccess;

TEST(bitset_tests, simple_test)
{
    BitsetStream bstream;

    std::vector<uint8_t> testdata;
    unsigned char data = 0x03;
    bstream.append(data);
    testdata.push_back(data);
    ASSERT_EQ(testdata, bstream.getData());

    data = 0x0F;
    bstream.append(data, 4);
    testdata.push_back(0xF0);
    ASSERT_THROW(bstream.append(data, 9), std::out_of_range);
    ASSERT_EQ(testdata, bstream.getData());

    ASSERT_FALSE(bstream.test(0));
    ASSERT_FALSE(bstream.test(1));
    ASSERT_FALSE(bstream.test(2));
    ASSERT_FALSE(bstream.test(3));
    ASSERT_FALSE(bstream.test(4));
    ASSERT_FALSE(bstream.test(5));
    ASSERT_TRUE(bstream.test(6));
    ASSERT_TRUE(bstream.test(7));
    ASSERT_TRUE(bstream.test(8));
    ASSERT_TRUE(bstream.test(9));
    ASSERT_TRUE(bstream.test(10));
    ASSERT_TRUE(bstream.test(11));

    ASSERT_EQ(bstream.toString(), "000000111111");
    ASSERT_EQ(bstream.toString(6, 11), "111111");
    ASSERT_EQ(bstream.any(), true);
    ASSERT_EQ(bstream.none(), false);
    ASSERT_EQ(bstream.all(), false);

    data = 0x3C;
    bstream.append(data, 2, 4);
    testdata.pop_back();
    testdata.push_back(0xFF);
    ASSERT_THROW(bstream.append(data, 6, 7), std::out_of_range);
    ASSERT_EQ(testdata, bstream.getData());

    std::vector<uint8_t> data2;
    data2.push_back(0xCC);
    data2.push_back(0xC0);
    bstream.concat(data2);
    testdata.push_back(0xCC);
    testdata.push_back(0xC0);
    ASSERT_EQ(testdata, bstream.getData());

    data2.pop_back();
    data2.pop_back();
    data2.push_back(0x6D);
    data2.push_back(0xBC);
    bstream.concat(data2, 1, 13);
    testdata.push_back(0xDB);
    testdata.push_back(0x78);
    ASSERT_EQ(testdata, bstream.getData());

    std::vector<uint8_t> testdata2;

    testdata2.push_back(0x03);
    testdata2.push_back(0xF3);
    testdata2.push_back(0xBF);
    testdata2.push_back(0xCC);
    testdata2.push_back(0xC0);
    testdata2.push_back(0xDB);
    testdata2.push_back(0x78);
    bstream.insert(11, 0x9D, 0, 8);
    ASSERT_EQ(testdata2, bstream.getData());

    std::vector<uint8_t> testdata3;

    testdata3.push_back(0x03);
    testdata3.push_back(0xF3);
    testdata3.push_back(0xBF);
    testdata3.push_back(0xCB);
    testdata3.push_back(0x6D);
    testdata3.push_back(0xE4);
    testdata3.push_back(0xC0);
    testdata3.push_back(0xDB);
    testdata3.push_back(0x78);
    bstream.insert(29, data2, 0, 16);
    ASSERT_EQ(testdata3, bstream.getData());

    std::vector<uint8_t> testdata4;

    testdata4.push_back(0x03);
    testdata4.push_back(0xF3);
    testdata4.push_back(0x97);
    testdata4.push_back(0xCB);
    testdata4.push_back(0x6D);
    testdata4.push_back(0xE4);
    testdata4.push_back(0xC0);
    testdata4.push_back(0xDB);
    testdata4.push_back(0x78);
    bstream.writeAt(16, 0x12, 3, 5);
    ASSERT_THROW(bstream.writeAt(345, 0x0F, 4, 6), std::out_of_range);
    ASSERT_EQ(testdata4, bstream.getData());
}

TEST(bitset_tests, complete_test)
{
    BitsetStream bstream;

    ASSERT_THROW(bstream.append(10, 9), std::out_of_range);
    ASSERT_THROW(bstream.append(10, 2, 13), std::out_of_range);
    bstream.append(10, 8, 0);
    ASSERT_EQ(0, bstream.getData().size());
    bstream.append(0);
    bstream.append(0);
    ASSERT_EQ(2, bstream.getData().size());
    bstream.clear();
    ASSERT_EQ(0, bstream.getData().size());
    bstream.append(0);
    bstream.append(100);
    ASSERT_EQ(2, bstream.getData().size());
    bstream.clear();
    bstream.append(0);
    bstream.append(4);
    ASSERT_EQ(2, bstream.getData().size());
    bstream.clear();
    bstream.append(4);
    bstream.append(2);
    ASSERT_EQ(42, bstream.getData()[0] * 10 + bstream.getData()[1]);
    bstream.clear();
    bstream.append(4);
    bstream.append(0);
    bstream.append(2);
    ASSERT_EQ(402, bstream.getData()[0] * 100 + bstream.getData()[1] * 10 +
                       bstream.getData()[2]);
    bstream.clear();
    bstream.append(4, 1, 2);
    ASSERT_EQ(bstream.getData()[0], 0);
    bstream.clear();
    bstream.append(4, 6, 2);
    ASSERT_EQ(bstream.getData()[0], 0);
    bstream.clear();
    bstream.append(4, 5, 3);
    ASSERT_EQ(bstream.getData()[0], 128);
    bstream.clear();
    bstream.append(4, 6, 2);
    ASSERT_EQ(bstream.getData()[0], 0);
    bstream.clear();
    bstream.append(4, 1, 2);
    ASSERT_EQ(bstream.getData()[0], 0);

    std::vector<uint8_t> a, b;
    a.push_back(1);
    a.push_back(16);
    b.push_back(5);
    b.push_back(32);

    bstream.clear();
    bstream.concat(a);
    ASSERT_EQ(bstream.getData()[0] * 10 + bstream.getData()[1], 26);
    bstream.concat(b);
    ASSERT_EQ(bstream.getData()[0] * 1000 + bstream.getData()[1] * 100 +
                  bstream.getData()[2] * 10 + bstream.getData()[3],
              2682);
    ASSERT_NO_THROW(bstream.concat(std::vector<uint8_t>()));
    bstream.clear();
    bstream.concat(a, 1, 1);
    ASSERT_EQ(bstream.getData()[0], 0);
    bstream.concat(b, 5, 3);
    ASSERT_EQ(bstream.getData()[0], 80);

    bstream.clear();
    ASSERT_THROW(bstream.insert(2, 0x0F), std::out_of_range);
    ASSERT_THROW(bstream.insert(0, std::vector<uint8_t>(), 0, 10), std::invalid_argument);
    ASSERT_THROW(bstream.insert(0, a, 17, 4), std::out_of_range);

    a.clear();
    bstream.insert(0, 0xDA);
    a.push_back(0xDA);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(4, 0x0B, 4, 4);
    a.clear();
    a.push_back(0xDB);
    a.push_back(0xA0);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(0, 0xBB);
    a.insert(a.begin(), 0xBB);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(6, 0xFA);
    a[1] = 0xEB;
    a[2] = 0xDB;
    a.push_back(0xA0);
    ASSERT_EQ(a, bstream.getData());
    bstream.append(0x01);
    a.push_back(0x10);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(13, 0x01, 7, 1);
    a[1] = 0xED;
    a[2] = 0xED;
    a[3] = 0xD0;
    a[4] = 0x08;
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(37, 0x00, 1, 1);
    ASSERT_EQ(38, bstream.getBitSize());
    bstream.clear();
    bstream.insert(0, a, 0, static_cast<unsigned int>(a.size() * 8));
    ASSERT_EQ(a, bstream.getData());

    bstream.clear();
    ASSERT_THROW(bstream.writeAt(2, 0x0F), std::out_of_range);
    ASSERT_THROW(bstream.writeAt(0, std::vector<uint8_t>(), 0, 10),
                 std::invalid_argument);
    ASSERT_THROW(bstream.writeAt(0, a, 17, 4), std::out_of_range);
    ASSERT_THROW(bstream.writeAt(0, 0), std::out_of_range);

    a.clear();
    bstream.append(0);
    bstream.writeAt(0, 0xDA);
    a.push_back(0xDA);
    ASSERT_EQ(a, bstream.getData());
    bstream.append(0xA0);
    bstream.writeAt(4, 0x0B, 4, 4);
    a.clear();
    a.push_back(0xDB);
    a.push_back(0xA0);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(0, 0x00);
    bstream.writeAt(0, 0xBB);
    a.insert(a.begin(), 0xBB);
    ASSERT_EQ(a, bstream.getData());
    bstream.insert(6, 0x00);
    bstream.writeAt(6, 0xFA);
    a[1] = 0xEB;
    a[2] = 0xDB;
    a.push_back(0xA0);
    ASSERT_EQ(a, bstream.getData());
    bstream.clear();
    bstream.append(0);
    bstream.append(0);
    bstream.append(0);
    bstream.append(0);
    a.clear();
    a.push_back(0x01);
    a.push_back(0x23);
    a.push_back(0x45);
    a.push_back(0x67);
    bstream.writeAt(0, a, 0, static_cast<unsigned int>(a.size() * 8));
    ASSERT_EQ(a, bstream.getData());

    bstream.clear();
    bstream.append(0xFF);
    bstream.append(0xFF);
    bstream.append(0xFF);
    bstream.append(0xFF);
    ASSERT_EQ(bstream.toULong(), ULONG_MAX);
    bstream.append(0xFF);
    ASSERT_THROW(bstream.toULong(), std::overflow_error);
    bstream.append(0xFF);
    bstream.append(0xFF);
    bstream.append(0xFF);
    ASSERT_EQ(bstream.toULLong(), ULLONG_MAX);
    bstream.append(0xFF);
    ASSERT_THROW(bstream.toULLong(), std::overflow_error);
}

TEST(bitset_tests, append_4_bits)
{
    BitsetStream bs;

    bs.append(1, 4, 4);
    bs.append(1, 4, 4);
    ASSERT_EQ(0b00010001, bs.getData().at(0));
}

TEST(bitset_tests, append_4_bits_2)
{
    BitsetStream bs;

    bs.append(2, 4, 4);
    bs.append(3, 4, 4);
    ASSERT_EQ(0b00100011, bs.getData().at(0));
}

TEST(bitset_tests, append_x_bits)
{
    BitsetStream bs;

    bs.append(0b00000001, 7, 1);
    bs.append(0b11111110, 0, 7);
    ASSERT_EQ(0b11111111, bs.getData().at(0));
}
