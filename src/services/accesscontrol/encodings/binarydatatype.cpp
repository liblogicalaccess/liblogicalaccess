/**
 * \file binarydatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Binary Data Type.
 */

#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#include <cstring>

namespace logicalaccess
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

    unsigned int BinaryDataType::convert(unsigned long long data, unsigned int dataLengthBits, BitsetStream& dataConverted)
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
        unsigned int tmpswblen = DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, BitsetStream(), BitsetStream());
        size_t tmpswblenBytes = (tmpswblen + 7) / 8;
		BitsetStream tmpswb(0x00, tmpswblenBytes);
		
		BitsetStream tmpdata;
		std::vector<uint8_t> tmpvdata;
		tmpvdata.assign(reinterpret_cast<uint8_t*>(&data), reinterpret_cast<uint8_t*>(&data) + sizeof(data));
		tmpdata.concat(tmpvdata);

        ret = BitHelper::align(tmpswb, tmpdata, tmpdata.getBitSize());
        DataType::addParityToBuffer(d_leftParityType, d_rightParityType, 8, tmpswb, dataConverted);
        //}

        return ret;
    }

	unsigned long long BinaryDataType::revert(BitsetStream& data)
    {
        unsigned long long ret = 0;

        if (data.getByteSize() > 0)
        {
            unsigned int tmpswblen = DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, BitsetStream(), BitsetStream());
            size_t tmpswblenBytes = (tmpswblen + 7) / 8;
            //unsigned char* tmpswb = new unsigned char[tmpswblenBytes];
            //memset(tmpswb, 0x00, tmpswblenBytes);
			BitsetStream tmpswb(0x00, tmpswblenBytes);

            DataType::removeParityToBuffer(d_leftParityType, d_rightParityType, 8, data, tmpswb);

            //unsigned char *linedData = new unsigned char[dataLengthBytes];
            //memset(linedData, 0x00, dataLengthBytes);
			BitsetStream linedData(0x00, data.getByteSize());
            BitHelper::revert(linedData, tmpswb, tmpswblen);

            int i = 0;
            Alg(data.getByteSize(), &i);

            for (long long n = (static_cast<long long>(data.getByteSize() - 1) * 8); n >= 0; n -= 8) // TODO: Check this loop
            {
                Alg(&i);
                if (d_bitDataRepresentationType == ET_LITTLEENDIAN)
                {
                    linedData.writeAt(i, DataType::invertBitSex(linedData.getData()[i]));
                }
                ret |= (static_cast<unsigned long long>(linedData.getData()[i]) & 0xff) << n;
            }
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