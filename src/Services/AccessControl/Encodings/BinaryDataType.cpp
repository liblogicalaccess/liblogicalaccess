/**
 * \file BinaryDataType.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Binary Data Type.
 */

#include "logicalaccess/Services/AccessControl/Encodings/BinaryDataType.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include <cstring>

namespace LOGICALACCESS
{
	BinaryDataType::BinaryDataType()
		: DataType()
	{
	}

	BinaryDataType::~BinaryDataType()
	{
	}

	string BinaryDataType::getName() const
	{
		return string("Binary");
	}

	EncodingType BinaryDataType::getType() const
	{
		return ET_BINARY;
	}

	unsigned int BinaryDataType::convert(unsigned long long data, unsigned int dataLengthBits, void* dataConverted, size_t dataConvertedLengthBytes)
	{
		unsigned int ret = 0;

		/*if (dataLengthBits <= 8)
		{
			ret = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, NULL, dataLengthBits, NULL, 0);

			if (dataConverted != NULL)
			{
				size_t datalengthBytes = (dataLengthBits + 7) / 8;
				unsigned char* tmpswb = new unsigned char[datalengthBytes];				
				memcpy(tmpswb, &data, datalengthBytes);

				if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
				{
					for (unsigned int i = 0; i < datalengthBytes; ++i)
					{
						tmpswb[i] = DataType::invertBitSex(tmpswb[i]);
					}
				}

				memset(dataConverted, 0x00, dataConvertedLengthBytes);			
				DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, tmpswb, dataLengthBits, dataConverted, dataConvertedLengthBytes * 8);
				delete[] tmpswb;
			}
		}
		else
		{*/
			unsigned int tmpswblen = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, NULL, static_cast<unsigned int>(dataConvertedLengthBytes * 8), NULL, 0);
			size_t tmpswblenBytes = (tmpswblen + 7) / 8;
			unsigned char* tmpswb = new unsigned char[tmpswblenBytes];
			memset(tmpswb, 0x00, tmpswblenBytes);
			ret = BitHelper::align(tmpswb, tmpswblenBytes, &data, sizeof(data), dataLengthBits);
			DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, tmpswb, tmpswblen, dataConverted, static_cast<unsigned int>(dataConvertedLengthBytes * 8));
			delete[] tmpswb;
		//}

		return ret;
	}

	unsigned long long BinaryDataType::revert(void* data, size_t dataLengthBytes, unsigned int lengthBits)
	{
		unsigned long long ret = 0;

		if (data != NULL && dataLengthBytes > 0)
		{
			unsigned int tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, NULL, lengthBits, NULL, 0);
			size_t tmpswblenBytes = (tmpswblen + 7) / 8;
			unsigned char* tmpswb = new unsigned char[tmpswblenBytes];
			memset(tmpswb, 0x00, tmpswblenBytes);

			DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, data, lengthBits, tmpswb, tmpswblen);

			unsigned char *linedData = new unsigned char[dataLengthBytes];
			memset(linedData, 0x00, dataLengthBytes);
			BitHelper::revert(linedData, dataLengthBytes, tmpswb, tmpswblenBytes, tmpswblen);
			delete[] tmpswb;

			int i = 0;
			Alg(static_cast<int>(dataLengthBytes), &i);

			for (long long n = (static_cast<long long>(dataLengthBytes - 1 ) * 8); n >= 0; n -= 8) // TODO: Check this loop
			{
				Alg(&i);
				if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
				{	
					linedData[i] = DataType::invertBitSex(linedData[i]);
				}
				ret |= (static_cast<unsigned long long>(linedData[i]) & 0xff) << n;
			}
			delete[] linedData;
		}
		return ret;
	}
			
	void BinaryDataType::Alg(int s, int* i)
	{
		#if __BYTE_ORDER == __LITTLE_ENDIAN

			*i = s;

		#else

			*i = -1;

		#endif
	}

	void BinaryDataType::Alg(int* i)
	{
		#if __BYTE_ORDER == __LITTLE_ENDIAN

			--(*i);

		#else

			++(*i);

		#endif
	}
}
