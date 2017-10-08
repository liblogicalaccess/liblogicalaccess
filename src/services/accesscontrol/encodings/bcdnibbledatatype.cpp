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

    unsigned int BCDNibbleDataType::convert(unsigned long long data, const unsigned int dataLengthBits, void* dataConverted, const size_t dataConvertedLengthBytes)
    {
	    unsigned char* tmp = new unsigned char[64];
        memset(tmp, 0x00, 64);

        unsigned int shft, i;

        for (shft = 0, i = 0; shft < dataLengthBits; shft += 4, ++i)
        {
	        const unsigned int offset = ((i % 2) == 0) ? 0 : 4;
            unsigned char c = static_cast<unsigned char>(data % 10);
            if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
            {
                c = invertBitSex(c, 4);
            }
            c = static_cast<unsigned char>(c << offset);
            tmp[i / 2] |= c;
            data /= 10;
        }

        i = (i + 1) / 2;

	    auto ret = addParityToBuffer(d_leftParityType, d_rightParityType, 4, nullptr, shft, nullptr, 0);

        if (dataConverted != nullptr)
        {
            if (dataConvertedLengthBytes >= i)
            {
	            const auto swb = reinterpret_cast<unsigned char*>(dataConverted);
	            const auto tmpswb = new unsigned char[i];
	            const auto tmpswb2 = new unsigned char[i];
                memset(tmpswb, 0x00, i);
                memset(tmpswb2, 0x00, i);

                for (size_t pi = i - 1, p = 0; p < i; --pi, ++p)
                {
                    tmpswb[p] = tmp[pi];
                }

                ret = BitHelper::align(tmpswb2, i, tmpswb, i, dataLengthBits);
                addParityToBuffer(d_leftParityType, d_rightParityType, 4, tmpswb2, shft, swb, static_cast<unsigned int>(dataConvertedLengthBytes * 8));
                delete[] tmpswb;
                delete[] tmpswb2;
            }
            else
            {
                //too small
            }
        }

        delete[] tmp;

        return ret;
    }

    unsigned long long BCDNibbleDataType::revert(void* data, const size_t dataLengthBytes, const unsigned int lengthBits)
    {
        unsigned long long ret = 0;

        if (data != nullptr && dataLengthBytes > 0)
        {
	        const unsigned int tmpswblen = removeParityToBuffer(d_leftParityType, d_rightParityType, 4, nullptr, lengthBits, nullptr, 0);
	        const size_t tmpswblenBytes = (tmpswblen + 7) / 8;
            unsigned char* tmpswb = new unsigned char[tmpswblenBytes];
            memset(tmpswb, 0x00, tmpswblenBytes);

            removeParityToBuffer(d_leftParityType, d_rightParityType, 4, data, lengthBits, tmpswb, static_cast<unsigned int>(tmpswblenBytes * 8));

            size_t i;
            int coef;

            for (i = 0, coef = static_cast<int>(((tmpswblen + 3) / 4) - 1); i < tmpswblenBytes; ++i, coef -= 2)
            {
                if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
                {
                    tmpswb[i] = ((0x0F & invertBitSex(tmpswb[i] >> 4, 4)) << 4) | (tmpswb[i] & 0x0F);
                    tmpswb[i] = (0x0F & invertBitSex(tmpswb[i], 4)) | (tmpswb[i] & 0xF0);
                }
                ret += (((tmpswb[i] & 0xF0) >> 4) * (pow(10, coef)));
                if (coef > 0)
                {
                    ret += ((tmpswb[i] & 0x0F) * pow(10, (coef - 1)));
                }
            }

            delete[] tmpswb;
        }

        return ret;
    }
}