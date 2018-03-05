/**
 * \file bcdnibbledatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCD Nibble Data Type.
 */

#include <algorithm>
#include <logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

#include <cstring>

namespace logicalaccess
{
BCDNibbleDataType::BCDNibbleDataType()
    : DataType()
{
}

BCDNibbleDataType::~BCDNibbleDataType()
{
}

std::string BCDNibbleDataType::getName() const
{
    return std::string("BCD-Nibble");
}

EncodingType BCDNibbleDataType::getType() const
{
    return ET_BCDNIBBLE;
}


/**
 * Return the number of digits in a given number.
 */
static int nb_digits(unsigned long long nb)
{
    if (nb == 0)
        return 1;

    int count = 0;
    while (nb != 0)
    {
        nb /= 10;
        ++count;
    }

    return count;
}

/**
 * Returns the DECIMAL DIGITS of a number, starting from left to right.
 *
 * If the number is 12345, this will return {1, 2, 3, 4, 5}
 */
std::vector<uint8_t> get_digits(unsigned long long nb)
{
    std::vector<uint8_t> digits;
    int initial_nb_digits = nb_digits(nb);

    for (int i = 0; i < initial_nb_digits; ++i)
    {
        auto digit = static_cast<uint8_t>(nb % 10);
        digits.push_back(digit);
        nb /= 10;
    }

    std::reverse(digits.begin(), digits.end());
    return digits;
}

BitsetStream BCDNibbleDataType::convert(unsigned long long data,
                                        unsigned int dataLengthBits)
{
    BitsetStream bs;

    // We convert by appending digit by digit into the bitsetstream.
    // Each digit is encoded over 4 bits.

    // Pad with 0 if expected length > number of digits in number.
    int nb_digits_from_length = dataLengthBits / 4;
    if (nb_digits_from_length > nb_digits(data))
    {
        for (int i = 0; i < nb_digits_from_length - get_digits(data).size(); ++i)
        {
            bs.append(0, 4, 4);
        }
    }

    for (uint8_t digit : get_digits(data))
    {
        if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
        {
            digit = DataType::invertBitSex(digit, 4);
        }

        // One digit is encoded with 4bits, so all high weight bits are 0
        // and we append the 4 lowest bits.
        bs.append(digit, 4, 4);
    }
    BitsetStream x = DataType::addParity(d_leftParityType, d_rightParityType, 4, bs);
    return x;
}

unsigned long long BCDNibbleDataType::revert(BitsetStream &data,
                                             unsigned int dataLengthBits)
{
    unsigned long long ret = 0;

    if (data.getByteSize() > 0)
    {
        BitsetStream tmpswb =
            DataType::removeParity(d_leftParityType, d_rightParityType, 4, data);


        for (int i = 0, coef = static_cast<int>(((tmpswb.getBitSize() + 3) / 4) - 1);
             static_cast<size_t>(i) < tmpswb.getByteSize(); ++i, coef -= 2)
        {
            if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
            {
                tmpswb.writeAt(
                    i * 8,
                    ((0x0F & DataType::invertBitSex(tmpswb.getData()[i] >> 4, 4)) << 4) |
                        (tmpswb.getData()[i] & 0x0F));
                tmpswb.writeAt(i * 8,
                               (0x0F & DataType::invertBitSex(tmpswb.getData()[i], 4)) |
                                   (tmpswb.getData()[i] & 0xF0));
            }
            ret += (((tmpswb.getData()[i] & 0xF0) >> 4) * (pow(10, coef)));
            if (coef > 0)
            {
                ret += ((tmpswb.getData()[i] & 0x0F) * pow(10, (coef - 1)));
            }
        }
    }

    return ret;
}
}