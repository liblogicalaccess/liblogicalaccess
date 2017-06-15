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

    string BCDNibbleDataType::getName() const
    {
        return string("BCD-Nibble");
    }

    EncodingType BCDNibbleDataType::getType() const
    {
        return ET_BCDNIBBLE;
    }

    unsigned int BCDNibbleDataType::convert(unsigned long long data, unsigned int dataLengthBits, BitsetStream& dataConverted)
    {
        unsigned int ret = 0;

        //unsigned char* tmp = new unsigned char[64];
        //memset(tmp, 0x00, 64);
		BitsetStream tmp(0x00, 64);

        unsigned int shft, i;

        for (shft = 0, i = 0; shft < dataLengthBits; shft += 4, ++i)
        {
            unsigned int offset = ((i % 2) == 0) ? 0 : 4;
            unsigned char c = (unsigned char)(data % 10);
            if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
            {
                c = DataType::invertBitSex(c, 4);
            }
            c = (unsigned char)(c << offset);
            tmp.writeAt((i * 8) / 2, tmp.getData()[i / 2] | c);
            data /= 10;
        }

        i = (i + 1) / 2;

        ret = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 4, BitsetStream(), BitsetStream());

        if (dataConverted.getByteSize() >= i)
        {
            //unsigned char* swb = reinterpret_cast<unsigned char*>(dataConverted);
            //unsigned char* tmpswb = new unsigned char[i];
            //unsigned char* tmpswb2 = new unsigned char[i];
            //memset(tmpswb, 0x00, i);
            //memset(tmpswb2, 0x00, i);
			BitsetStream tmpswb(0x00, i);
			BitsetStream tmpswb2(0x00, i);

            for (size_t pi = i - 1, p = 0; p < i; --pi, ++p)
            {
                tmpswb.writeAt(p * 8, tmp.getData()[pi]);
            }

            ret = BitHelper::align(tmpswb2, tmpswb, dataLengthBits);
            DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 4, tmpswb2, dataConverted);
        }
        else
        {
            //too small
        }
        return ret;
    }

    unsigned long long BCDNibbleDataType::revert(BitsetStream& data)
    {
        unsigned long long ret = 0;

        if (data.getByteSize() > 0)
        {
            unsigned int tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 4, BitsetStream(), BitsetStream());
            size_t tmpswblenBytes = (tmpswblen + 7) / 8;
            //unsigned char* tmpswb = new unsigned char[tmpswblenBytes];
            //memset(tmpswb, 0x00, tmpswblenBytes);
			BitsetStream tmpswb(0x00, tmpswblenBytes);

            DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 4, data, tmpswb);

            size_t i;
            int coef;

            for (i = 0, coef = static_cast<int>(((tmpswblen + 3) / 4) - 1); i < tmpswblenBytes; ++i, coef -= 2)
            {
                if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
                {
                    tmpswb.writeAt(i * 8,((0x0F & DataType::invertBitSex(tmpswb.getData()[i] >> 4, 4)) << 4) | (tmpswb.getData()[i] & 0x0F));
                    tmpswb.writeAt(i * 8,(0x0F & DataType::invertBitSex(tmpswb.getData()[i], 4)) | (tmpswb.getData()[i] & 0xF0));
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