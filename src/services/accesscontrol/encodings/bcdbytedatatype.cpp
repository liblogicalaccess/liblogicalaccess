/**
 * \file bcdbytedatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief BCD Byte Data Type.
 */

#include "logicalaccess/services/accesscontrol/encodings/bcdbytedatatype.hpp"

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

    string BCDByteDataType::getName() const
    {
        return string("BCD-Byte");
    }

    EncodingType BCDByteDataType::getType() const
    {
        return ET_BCDBYTE;
    }

    unsigned int BCDByteDataType::convert(unsigned long long data, unsigned int dataLengthBits, BitsetStream& dataConverted)
    {
        unsigned int ret = 0;

		BitsetStream tmp(0x00, 64);

        unsigned int shft, i;

        for (shft = 0, i = 0; shft < dataLengthBits; shft += 8, ++i)
        {
            tmp.writeAt(i, (unsigned char)(data % 10));
            data /= 10;
        }

        ret = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, BitsetStream(), BitsetStream());

		if (dataConverted.getByteSize() >= i)
		{
			BitsetStream tmpswb(0x00, i);

            for (size_t pi = i - 1, p = 0; p < i; --pi, ++p)
            {
                if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
                {
                    tmp.writeAt(pi, DataType::invertBitSex(tmp.getData()[pi]));
                }
                tmpswb.writeAt(p, tmp.getData()[pi]);
            }

            DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, tmpswb, dataConverted);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The destination buffer is too small.");
        }
        return ret;
    }

    unsigned long long BCDByteDataType::revert(BitsetStream& data)
    {
        unsigned long long ret = 0;

        if (data.getByteSize() > 0)
        {
            size_t tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, BitsetStream(), BitsetStream());
            size_t tmpswblenBytes = (tmpswblen + 7) / 8;
			BitsetStream tmpswb(0x00, tmpswblenBytes);

            DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, data, tmpswb);

            size_t i;
            int coef;

            for (i = 0, coef = static_cast<int>(data.getByteSize() - 1); i < data.getByteSize(); ++i, --coef)
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