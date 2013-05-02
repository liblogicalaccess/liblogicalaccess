/**
 * \file BCDByteDataType.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief BCD Byte Data Type.
 */

#include "logicalaccess/Services/AccessControl/Encodings/BCDByteDataType.h"

#include <cstring>

namespace LOGICALACCESS
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

	unsigned int BCDByteDataType::convert(unsigned long long data, unsigned int dataLengthBits, void* dataConverted, size_t dataConvertedLengthBytes)
	{
		unsigned int ret = 0;

		unsigned char* tmp = new unsigned char[64];
		memset(tmp, 0x00, 64);

		unsigned int shft, i;

		for (shft = 0, i = 0; shft < dataLengthBits; shft += 8, ++i)
		{
			tmp[i] = (unsigned char)(data % 10);
			data /= 10;
		}

		ret = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, NULL, i * 8, NULL, 0);

		if (dataConverted != NULL)
		{
			if (dataConvertedLengthBytes >= i)
			{
				unsigned char* swb = reinterpret_cast<unsigned char*>(dataConverted);
				unsigned char* tmpswb = new unsigned char[i];
				memset(tmpswb, 0x00, i);

				for (size_t pi = i - 1, p = 0; p < i; --pi, ++p)
				{
					if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
					{
						tmp[pi] = DataType::invertBitSex(tmp[pi]);
					}
					tmpswb[p] = tmp[pi];
				}

				DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, tmpswb, i * 8, swb, static_cast<unsigned int>(dataConvertedLengthBytes * 8));
				delete[] tmpswb;
			}
			else
			{
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The destination buffer is too small.");
			}
		}

		delete[] tmp;

		return ret;
	}

	unsigned long long BCDByteDataType::revert(void* data, size_t dataLengthBytes, unsigned int lengthBits)
	{
		unsigned long long ret = 0;

		if (data != NULL && dataLengthBytes > 0)
		{
			size_t tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, NULL, lengthBits, NULL, 0);
			size_t tmpswblenBytes = (tmpswblen + 7) / 8;
			unsigned char* tmpswb = new unsigned char[tmpswblenBytes];		
			memset(tmpswb, 0x00, tmpswblenBytes);

			DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, data, lengthBits, tmpswb, static_cast<unsigned int>(tmpswblenBytes * 8));

			size_t i;
			int coef;

			for (i = 0, coef = static_cast<int>(dataLengthBytes - 1); i < dataLengthBytes; ++i, --coef)
			{
				if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
				{
					tmpswb[i] = DataType::invertBitSex(tmpswb[i]);
				}

				ret += (tmpswb[i] * ((coef == 0) ? 1 : pow(10, coef)));
			}		

			delete[] tmpswb;
		}

		return ret;
	}
}

