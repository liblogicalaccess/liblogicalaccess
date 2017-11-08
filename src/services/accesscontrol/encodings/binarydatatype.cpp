/**
 * \file binarydatatype.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Binary Data Type.
 */

#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

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

std::string BinaryDataType::getName() const
{
    return std::string("Binary");
}

EncodingType BinaryDataType::getType() const
{
    return ET_BINARY;
}

BitsetStream BinaryDataType::convert(unsigned long long data, unsigned int dataLengthBits)
{
    BitsetStream tmpdata;
    ByteVector tmpvdata;
    size_t datalengthBytes = (dataLengthBits + 7) / 8;
    tmpvdata.assign(reinterpret_cast<uint8_t *>(&data),
                    reinterpret_cast<uint8_t *>(&data) + datalengthBytes);
    tmpdata.concat(tmpvdata);

    BitsetStream tmpswb = BitHelper::align(tmpdata, dataLengthBits);
    BitsetStream dataConverted =
        DataType::addParity(d_leftParityType, d_rightParityType, 8, tmpswb);

    return dataConverted;
}

unsigned long long BinaryDataType::revert(BitsetStream &data, unsigned int dataLengthBits)
{
    unsigned long long ret = 0;

    if (data.getByteSize() > 0)
    {
        BitsetStream tmpswb =
            DataType::removeParity(d_leftParityType, d_rightParityType, 8, data);

        BitsetStream linedData = BitHelper::revert(tmpswb, dataLengthBits);

        int i = 0;
        Alg(data.getByteSize(), &i);

        for (long long n = (static_cast<long long>(data.getByteSize() - 1) * 8); n >= 0;
             n -= 8) // TODO: Check this loop
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

void BinaryDataType::Alg(int s, int *i)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

    *i = s;

#else

    *i = -1;

#endif
}

void BinaryDataType::Alg(int *i)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

    --(*i);

#else

    ++(*i);

#endif
}
}