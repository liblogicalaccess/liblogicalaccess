/**
 * \file bithelper.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief BitHelper Base.
 */

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/logs.hpp"
#include <cstring>

namespace logicalaccess
{
    unsigned int BitHelper::align(BitsetStream& linedData, const BitsetStream& data, unsigned int dataLengthBits)
    {
        unsigned int ret = 0;
        unsigned int offset = 8 - (dataLengthBits % 8);

		if ((ret = truncateLittleEndian(linedData, data, dataLengthBits)) > 0)
		{
			if (linedData.getByteSize() >= ((ret + 7) / 8))
			{
				//unsigned char* tmp = new unsigned char[linedDataLengthBytes];
				//memset(tmp, 0x00, linedDataLengthBytes);
				BitsetStream tmp(0x00, linedData.getByteSize());

				if (offset != 8)
				{
					for (size_t i = 0; i < linedData.getByteSize(); ++i)
					{
						tmp.writeAt(i, (tmp.getData()[i] | (linedData.getData()[i] << offset)));
						if (i != linedData.getByteSize() - 1)
						{
							tmp.writeAt(i + 1, (tmp.getData()[i + 1] | (linedData.getData()[i] >> (8 - offset))));
						}
					}
					linedData.writeAt(0, tmp.getData(), 0, tmp.getBitSize());
				}

#if __BYTE_ORDER != __LITTLE_ENDIAN
				//memset(tmp, 0x00, linedDataLengthBytes);
				//swapBytes(tmp, linedDataLengthBytes, linedData, linedDataLengthBytes, ret);
				//memcpy(linedData, tmp, linedDataLengthBytes);
				BitsetStream temp(0x00, linedData.getByteSize());
				swapBytes(temp, linedData);
				linedData.writeAt(0, temp.getData(), 0, temp.getSize());
#endif
			}
		}
        return ret;
    }

    unsigned int BitHelper::revert(BitsetStream& revertedData, const BitsetStream& data, unsigned int dataLengthBits)
    {
        unsigned int ret = 0;
        unsigned int offset = 8 - (dataLengthBits % 8);

        if ((ret = truncateLittleEndian(revertedData, data, dataLengthBits)) > 0)
		{
			if (revertedData.getByteSize() >= ret)
			{
				//unsigned char* tmp = new unsigned char[revertedDataLengthBytes];
				//memset(tmp, 0x00, revertedDataLengthBytes);
				BitsetStream tmp(0x00, revertedData.getByteSize());

				if (offset != 8)
				{
					//unsigned char* reb = reinterpret_cast<unsigned char*>(revertedData);

					for (long long i = static_cast<long long>(revertedData.getByteSize() - 1); i >= 0; --i) // TODO: Check this loop
					{
						tmp.writeAt((long long)i, (tmp.getData()[i] | 0xff & (revertedData.getData()[i] >> offset)));

						if (i != 0)
						{
							tmp.writeAt((long long)i - 1, (tmp.getData()[i - 1] | 0xff & (revertedData.getData()[i] << (8 - offset))));
						}
					}
					//memcpy(revertedData, tmp, revertedDataLengthBytes);
					revertedData.writeAt(0, tmp.getData(), 0, tmp.getBitSize());
				}

#if __BYTE_ORDER != __LITTLE_ENDIAN
				//memset(tmp, 0x00, revertedDataLengthBytes);
				//swapBytes(tmp, revertedDataLengthBytes, revertedData, revertedDataLengthBytes, ret);
				//memcpy(revertedData, tmp, revertedDataLengthBytes);
				BitsetStream temp(0x00, revertedData.getByteSize());
				swapBytes(temp, revertedData);
				revertedData.writeAt(0, temp.getData(), 0, revertedData.getByteSize());
				
#endif
			}
        }
        return ret;
    }

    unsigned int BitHelper::truncateLittleEndian(BitsetStream& truncatedData, const BitsetStream& data, unsigned int dataLengthBits)
    {
        unsigned int ret = dataLengthBits;
        size_t copyLength = (dataLengthBits + 7) / 8;

        if (truncatedData.getByteSize() >= copyLength && data.getByteSize() >= copyLength)
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            //memcpy(truncatedData, data, copyLength);
			truncatedData.writeAt(0, data.getData(), 0, data.getBitSize());
#else
            for (size_t i = data.getData.size() - 1, p = 0; p < copyLength && i < SIZE_T_MAX; --i, ++p)
            {
                truncatedData.writeAt(p, data.getData()[i]);
            }
#endif
        }
        return ret;
    }

    void BitHelper::swapBytes(BitsetStream& swapedDatas, const BitsetStream& data)
    {
        if (swapedDatas.getData().size() >= data.getByteSize())
        {
            for (long long j = 0, i = static_cast<long long>(data.getByteSize() - 1); i >= 0; --i, ++j)
            {
				swapedDatas.writeAt((long long)j, data.getData()[i]);
            }
        }
    }

    unsigned int BitHelper::extract(BitsetStream& extractData, const BitsetStream& data, unsigned int readPosBits, unsigned int readLengthBits)
	{
        unsigned int dataWritten = 0;

        if ((readPosBits + readLengthBits) <= data.getByteSize() * 8)
        {
            if (readLengthBits <= data.getByteSize() * 8)
            {
                extractData.concat(data.getData(), readPosBits, readLengthBits);
            }
            else
            {
				extractData.concat(data.getData());
            }
			dataWritten = readLengthBits;
        }

        return dataWritten;
    }

	
    //void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits)
    //{
    //    writeToBit(writtenData, writtenDataLengthBytes, writePosBits, data, dataLengthBytes, dataLengthBits, 0, dataLengthBits);
    //}
	//
    //void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, const void* data, size_t /*dataLengthBytes*/, unsigned int /*dataLengthBits*/, unsigned int readPosBits, unsigned int readLengthBits)
    //    //void BitHelper::writeToBit(void* data, size_t dataLength, size_t* pos, const void* d, size_t dPos, size_t length)
    //{
    //    unsigned int blen;
    //    for (unsigned int i = readPosBits; i < (readPosBits + readLengthBits); i += blen)
    //    {
    //        unsigned int ofs = i % 8;
    //        blen = (8 - ofs);
    //        if ((i + blen) >(readLengthBits + readPosBits))
    //        {
    //            blen = (readLengthBits + readPosBits - i);
    //            /*if (blen > ofs)
    //            {
    //            blen -= ofs;
    //            }*/
    //        }
    //        writeToBit(writtenData, writtenDataLengthBytes, writePosBits, reinterpret_cast<const unsigned char*>(data)[i / 8], ofs, blen);
    //    }
    //}
	//
	//std::vector<uint8_t>  BitHelper::writeToBit(size_t writtenDataLengthBytes, unsigned int* writePosBits, unsigned char data)
    //{
    //    return writeToBit(writtenDataLengthBytes, writePosBits, data, 0, 8);
    //}
	//
	//std::vector<uint8_t>  BitHelper::writeToBit(size_t writtenDataLengthBytes, unsigned int* writePosBits, unsigned char data, unsigned int readPosBits, unsigned int readLengthBits)
    //{
    //    unsigned int block = (int)(*writePosBits / 8.0);
    //    unsigned int offset = (*writePosBits % 8);
    //    unsigned int notreadlen = 8 - (readLengthBits + readPosBits);
	//
	//	std::vector<uint8_t> datas;
	//
    //    if (offset > 0)
    //    {
    //        datas[block] |= 0xff & ((0xff & (((data >> notreadlen) << notreadlen) << readPosBits)) >> offset);
    //        if ((8 - offset) < readLengthBits)
    //        {
    //            if ((block + 1) >= writtenDataLengthBytes)
    //            {
    //                THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The result array is too short.");
    //            }
	//
    //            datas[block + 1] |= 0xff & (0xff & ((0xff & ((((data >> notreadlen) << notreadlen) << readPosBits))) << (8 - offset)));
    //        }
    //    }
    //    else
    //    {
    //        datas[block] |= (unsigned char)((unsigned char)(((unsigned char)(((data >> notreadlen) << notreadlen) << readPosBits))));
    //    }
	//
    //    (*writePosBits) += readLengthBits;
	//
	//	return datas;
    //}
	//
}