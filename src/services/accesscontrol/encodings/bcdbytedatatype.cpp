/**
 * \file bcdbytedatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCD Byte Data Type.
 */

#include <logicalaccess/services/accesscontrol/encodings/bcdbytedatatype.hpp>

#include <cstring>

namespace logicalaccess
{
BCDByteDataType::BCDByteDataType()
    : DataType()
{
}

BCDByteDataType::~BCDByteDataType()
{
}

std::string BCDByteDataType::getName() const
{
    return std::string("BCD-Byte");
}

EncodingType BCDByteDataType::getType() const
{
    return ET_BCDBYTE;
}

BitsetStream BCDByteDataType::convert(unsigned long long data,
                                      unsigned int dataLengthBits)
{

    BitsetStream tmp(64 * 8);

    unsigned int shft, i;

    for (shft = 0, i = 0; shft < dataLengthBits; shft += 8, ++i)
    {
        tmp.writeAt(i, (unsigned char)(data % 10));
        data /= 10;
    }
    BitsetStream buf;
    BitsetStream procbuf;
    procbuf = DataType::addParity(d_leftParityType, d_rightParityType, 8, buf);

    BitsetStream dataConverted(0x00, i);

    BitsetStream tmpswb(i * 8);

    for (unsigned int pi = i - 1, p = 0; p < i; --pi, ++p)
    {
        if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
        {
            tmp.writeAt(pi, DataType::invertBitSex(tmp.getData()[pi]));
        }
        tmpswb.writeAt(p, tmp.getData()[pi]);
    }

    dataConverted = DataType::addParity(d_leftParityType, d_rightParityType, 8, tmpswb);
    return dataConverted;
}

unsigned long long BCDByteDataType::revert(BitsetStream &data,
                                           unsigned int dataLengthBits)
{
    unsigned long long ret = 0;

    if (data.getByteSize() > 0)
    {
        BitsetStream tmpswb =
            DataType::removeParity(d_leftParityType, d_rightParityType, 8, data);

        int coef = static_cast<int>(data.getByteSize() - 1);
        for (unsigned int i = 0; i < data.getByteSize(); ++i, --coef)
        {
            if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
            {
                tmpswb.writeAt(i, DataType::invertBitSex(tmpswb.getData()[i]));
            }

            ret += (tmpswb.getData()[i] * ((coef == 0) ? 1 : pow(10, coef)));
        }
    }

    return ret;
}
}