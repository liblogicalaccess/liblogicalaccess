/**
 * \file bcdnibbledatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCD Nibble Data Type.
 */

#include "logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

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

    BitsetStream BCDNibbleDataType::convert(unsigned long long data, unsigned int dataLengthBits)
    {
        BitsetStream tmp((dataLengthBits / 4) * 8);

        unsigned int shft, i;


        for (shft = 0, i = 0; shft < dataLengthBits; shft += 4, ++i)
        {
            unsigned int offset = ((i % 2) == 0) ? 4 : 0;
            unsigned char c = (unsigned char)(data % 10);
            if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
            {
                c = DataType::invertBitSex(c, 4);
            }
           // c = (unsigned char)(c << offset);
            const unsigned char pos = i / 2;
           // tmp.append(c & 0x0F, 4, 4);
            tmp.writeAt((pos * 8) + offset, c & 0x0F, 4, 4);
            data /= 10;
        }

        i = (i + 1) / 2;

        BitsetStream tmpswb(0x00, i);

        for (unsigned int pi = i - 1, p = 0; p < i; --pi, ++p)
        {
            tmpswb.writeAt(p * 8, tmp.getData()[pi]);
        }

        BitsetStream tmpswb2 = BitHelper::align(tmpswb, dataLengthBits);
        BitsetStream dataConverted = DataType::addParity(d_leftParityType, d_rightParityType, 4, tmpswb2);

        return dataConverted;
    }

    unsigned long long BCDNibbleDataType::revert(BitsetStream& data, unsigned int dataLengthBits)
    {
        unsigned long long ret = 0;

        if (data.getByteSize() > 0)
        {
            BitsetStream tmpswb = DataType::removeParity(d_leftParityType, d_rightParityType, 4, data);


            for (int i = 0, coef = static_cast<int>(((tmpswb.getBitSize() + 3) / 4) - 1); static_cast<size_t>(i) < tmpswb.getByteSize(); ++i, coef -= 2)
            {
                if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
                {
                    tmpswb.writeAt(i * 8, ((0x0F & DataType::invertBitSex(tmpswb.getData()[i] >> 4, 4)) << 4) | (tmpswb.getData()[i] & 0x0F));
                    tmpswb.writeAt(i * 8, (0x0F & DataType::invertBitSex(tmpswb.getData()[i], 4)) | (tmpswb.getData()[i] & 0xF0));
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