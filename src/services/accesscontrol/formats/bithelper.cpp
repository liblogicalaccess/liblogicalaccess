/**
 * \file bithelper.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief BitHelper Base.
 */

#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/logs.hpp>
#include <cstring>
#include <logicalaccess/myexception.hpp>

#include <algorithm>

namespace logicalaccess
{
BitsetStream BitHelper::align(const BitsetStream &data, unsigned int dataLengthBits)
{
    unsigned int offset = 8 - (dataLengthBits % 8);
    BitsetStream linedData;

    if (data.getBitSize() > 0)
    {
        if (offset != 8)
        {
            auto dataVector = data.getData();
            std::reverse(dataVector.begin(), dataVector.end());

            BitsetStream linedData;
            linedData.concat(dataVector, offset, dataLengthBits);

            dataVector = linedData.getData();
            std::reverse(dataVector.begin(), dataVector.end());

            linedData.clear();
            linedData.concat(dataVector, 0, static_cast<unsigned int>(dataVector.size()) * 8);

            return linedData;
        }
    }
    return data;
}

BitsetStream BitHelper::revert(const BitsetStream &data, unsigned int dataLengthBits)
{
    unsigned int offset = 8 - (dataLengthBits % 8);

    if (data.getBitSize() > 0)
    {

        if (offset != 8)
        {
            auto dataVector = data.getData();
            ByteVector tmp(dataVector.size());
            for (long long i = static_cast<long long>(dataVector.size() - 1); i >= 0; --i)
            {
                tmp[i] |= 0xff & (dataVector[i] >> offset);

                if (i != 0)
                {
                    tmp[i - 1] |= 0xff & (dataVector[i] << (8 - offset));
                }
            }

            BitsetStream linedData;

            linedData.concat(tmp, 0, static_cast<unsigned int>(tmp.size()) * 8);

            return linedData;
        }
    }
    return data;
}

BitsetStream BitHelper::truncateLittleEndian(const BitsetStream &data,
                                             unsigned int dataLengthBits)
{
    BitsetStream truncatedData(dataLengthBits);

#if __BYTE_ORDER == __LITTLE_ENDIAN
    truncatedData.writeAt(0, data.getData(), 0, data.getBitSize());
#else
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "BitHelper::truncateLittleEndian NO IMPLEMENTED.");
    for (size_t i = data.getData.size() - 1, p = 0; p < copyLength && i < SIZE_T_MAX;
         --i, ++p)
    {
        truncatedData.writeAt(p * 8, data.getData()[i]);
    }
#endif
    return truncatedData;
}

BitsetStream BitHelper::swapBytes(const BitsetStream &data)
{
    auto byteRevert = data.getData();
    std::reverse(byteRevert.begin(), byteRevert.end());

    BitsetStream truncatedData;
    truncatedData.concat(byteRevert, 0, data.getByteSize() * 8);
    return truncatedData;
}

BitsetStream BitHelper::extract(const BitsetStream &data, unsigned int readPosBits,
                                unsigned int readLengthBits)
{
    BitsetStream extractData;

    if ((readPosBits + readLengthBits) <= data.getBitSize())
    {
        extractData.concat(data.getData(), readPosBits, readLengthBits);
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data is too small to extract this size.");

    return extractData;
}
}
