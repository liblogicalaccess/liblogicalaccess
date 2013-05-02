/**
 * \file BCDNibbleDataType.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief BCD Nibble Data Type.
 */

#include "logicalaccess/Services/AccessControl/Encodings/BCDNibbleDataType.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include <cstring>

namespace LOGICALACCESS
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

	unsigned int BCDNibbleDataType::convert(unsigned long long data, unsigned int dataLengthBits, void* dataConverted, size_t dataConvertedLengthBytes)
	{
		unsigned int ret = 0;

		unsigned char* tmp = new unsigned char[64];
		memset(tmp, 0x00, 64);

		unsigned int shft, i, offset;

		for (shft = 0, i = 0; shft < dataLengthBits; shft += 4, ++i)
		{
			offset = ((i % 2) ==  0) ? 0 : 4;
			unsigned char c = (unsigned char)(data % 10);
			if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
			{
				c = DataType::invertBitSex(c, 4);
			}
			c =  (unsigned char)(c << offset);
			tmp[i / 2] |= c;
			data /= 10;
		}

		i = (i + 1) / 2;

		ret = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 4, NULL, shft, NULL, 0);

		if (dataConverted != NULL)
		{
			if (dataConvertedLengthBytes >= i)
			{
				unsigned char* swb = reinterpret_cast<unsigned char*>(dataConverted);
				unsigned char* tmpswb = new unsigned char[i];
				unsigned char* tmpswb2 = new unsigned char[i];
				memset(tmpswb, 0x00, i);
				memset(tmpswb2, 0x00, i);

				for (size_t pi = i - 1, p = 0; p < i; --pi, ++p)
				{
					tmpswb[p] = tmp[pi];
				}
				
				ret = BitHelper::align(tmpswb2, i, tmpswb, i, dataLengthBits);
				DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 4, tmpswb2, shft, swb, static_cast<unsigned int>(dataConvertedLengthBytes * 8));
				delete[] tmpswb;
			}
			else
			{
				//too small
			}
		}

		delete[] tmp;

		return ret;
	}

	unsigned long long BCDNibbleDataType::revert(void* data, size_t dataLengthBytes, unsigned int lengthBits)
	{
		unsigned long long ret = 0;

		if (data != NULL && dataLengthBytes > 0)
		{
			unsigned int tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 4, NULL, lengthBits, NULL, 0);
			size_t tmpswblenBytes = (tmpswblen + 7) / 8;
			unsigned char* tmpswb = new unsigned char[tmpswblenBytes];		
			memset(tmpswb, 0x00, tmpswblenBytes);

			DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 4, data, lengthBits, tmpswb, static_cast<unsigned int>(tmpswblenBytes * 8));

			size_t i;
			int coef;

			for (i = 0, coef = static_cast<int>(((tmpswblen + 3) / 4) - 1); i < tmpswblenBytes; ++i, coef -= 2)
			{
				if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
				{
					tmpswb[i] = ((0x0F & DataType::invertBitSex(tmpswb[i] >> 4, 4)) << 4) | (tmpswb[i] & 0x0F);					
					tmpswb[i] = (0x0F & DataType::invertBitSex(tmpswb[i], 4)) | (tmpswb[i] & 0xF0);
				}				
				ret += (((tmpswb[i] & 0xF0) >> 4) * (pow(10, coef)));
				if (coef > 0)
				{
					ret += ((tmpswb[i] & 0x0F) * pow(10, (coef -1)));
				}
			}

			delete[] tmpswb;
		}

		return ret;
	}
}

