/**
 * \file BitHelper.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief BitHelper Base.
 */

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include <cstring>

namespace logicalaccess
{
	unsigned int BitHelper::align(void* linedData, size_t linedDataLengthBytes, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits)
	{
		unsigned int ret = 0;
		unsigned int offset = 8 - (dataLengthBits % 8);

		if ((ret = truncateLittleEndian(linedData, linedDataLengthBytes, data, dataLengthBytes, dataLengthBits)) > 0)
		{
			if (linedData != NULL)
			{
				if (linedDataLengthBytes >= ((ret + 7) / 8))
				{
					unsigned char* tmp = new unsigned char[linedDataLengthBytes];
					memset(tmp, 0x00, linedDataLengthBytes);

					if (offset != 8)
					{
						unsigned char* swb = reinterpret_cast<unsigned char*>(linedData);

						for (size_t i = 0; i < linedDataLengthBytes; ++i)
						{
							tmp[i] |= (unsigned char)(swb[i] << offset);
							if (i != linedDataLengthBytes - 1)
							{
								tmp[i + 1] |= (unsigned char)(swb[i] >> (8 - offset));
							}
						}
						memcpy(linedData, tmp, linedDataLengthBytes);
					}

					#if __BYTE_ORDER != __LITTLE_ENDIAN

						memset(tmp, 0x00, linedDataLengthBytes);
						swapBytes(tmp, linedDataLengthBytes, linedData, linedDataLengthBytes, ret);
						memcpy(linedData, tmp, linedDataLengthBytes);
					#endif

					delete[] tmp;
				}
			}
		}

		return ret;
	}

	unsigned int BitHelper::revert(void* revertedData, size_t revertedDataLengthBytes, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits)
	{
		unsigned int ret = 0;
		unsigned int offset = 8 - (dataLengthBits % 8);

		if ((ret = truncateLittleEndian(revertedData, revertedDataLengthBytes, data, dataLengthBytes, dataLengthBits)) > 0)
		{
			if (revertedData != NULL)
			{
				if ((revertedDataLengthBytes * 8) >= ret)
				{
					unsigned char* tmp = new unsigned char[revertedDataLengthBytes];
					memset(tmp, 0x00, revertedDataLengthBytes);

					if (offset != 8)
					{
						unsigned char* reb = reinterpret_cast<unsigned char*>(revertedData);

						for (long long i = static_cast<long long>(revertedDataLengthBytes - 1); i >= 0 ; --i) // TODO: Check this loop
						{
							tmp[i] |= 0xff & (reb[i] >> offset);

							if (i != 0)
							{
								tmp[i - 1] |= 0xff & (reb[i] << (8 - offset));
							}
						}

						memcpy(revertedData, tmp, revertedDataLengthBytes);
					}

					#if __BYTE_ORDER != __LITTLE_ENDIAN

						memset(tmp, 0x00, revertedDataLengthBytes);
						swapBytes(tmp, revertedDataLengthBytes, revertedData, revertedDataLengthBytes, ret);
						memcpy(revertedData, tmp, revertedDataLengthBytes);

					#endif

					delete[] tmp;
				}
			}
		}

		return ret;
	}

	unsigned int BitHelper::truncateLittleEndian(void* truncatedData, size_t truncatedDataLengthBytes, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits)
	{
		unsigned int ret = dataLengthBits;
		size_t copyLength = (dataLengthBits + 7) / 8;

		if (truncatedData != NULL && truncatedDataLengthBytes >= copyLength && dataLengthBytes >= copyLength)
		{
			#if __BYTE_ORDER == __LITTLE_ENDIAN

				memcpy(truncatedData, data, copyLength);

			#else

				unsigned char* dab = reinterpret_cast<unsigned char*>(data);
				unsigned char* trb = reinterpret_cast<unsigned char*>(truncatedData);

				for (size_t i = dataLengthBytes - 1, p = 0; p < copyLength && i >= 0; --i, ++p)
				{
					trb[p] = dab[i];
				}

			#endif
		}
		return ret;
	}

	void BitHelper::swapBytes(void* swapedData, size_t swapedDataLengthBytes, const void* data, size_t dataLengthBytes, unsigned int /*dataLengthBits*/)
	{
		if (swapedDataLengthBytes >= dataLengthBytes)
		{
			unsigned char* swb = reinterpret_cast<unsigned char*>(swapedData);
			const unsigned char* dab = reinterpret_cast<const unsigned char*>(data);

			for (long long j = 0, i = static_cast<long long>(dataLengthBytes - 1); i >= 0; --i, ++j)
			{
				swb[j] = dab[i];
			}
		}
	}

	unsigned int BitHelper::extract(void* extractData, size_t extractDataLengthBytes, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits, unsigned int readPosBits, unsigned int readLengthBits)
	{
		unsigned int dataWritten = 0;

		if (((readPosBits + readLengthBits) <= dataLengthBits) && (extractDataLengthBytes >= ((readLengthBits +7) / 8)))
		{
			if (readLengthBits <= dataLengthBits)
			{
				unsigned int rPos = 0;

				writeToBit(extractData, extractDataLengthBytes, &rPos, data, dataLengthBytes, dataLengthBits, readPosBits, readLengthBits);
				dataWritten = readLengthBits;
			}
			else
			{
				memcpy(extractData, data, dataLengthBytes);
				dataWritten = readLengthBits;
			}
		}

		return dataWritten;
	}

	void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, const void* data, size_t dataLengthBytes, unsigned int dataLengthBits)
	{
		writeToBit(writtenData, writtenDataLengthBytes, writePosBits, data, dataLengthBytes, dataLengthBits, 0, dataLengthBits);
	}

	void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, const void* data, size_t /*dataLengthBytes*/, unsigned int /*dataLengthBits*/, unsigned int readPosBits, unsigned int readLengthBits)
	//void BitHelper::writeToBit(void* data, size_t dataLength, size_t* pos, const void* d, size_t dPos, size_t length)
	{
		unsigned int ofs = 0;
		unsigned int blen;
		for (unsigned int i = readPosBits; i < (readPosBits + readLengthBits); i += blen)
		{
			ofs = i % 8;
			blen = (8 - ofs);
			if ((i + blen) > (readLengthBits + readPosBits))
			{
				blen = (readLengthBits + readPosBits - i);
				/*if (blen > ofs)
				{
					blen -= ofs;
				}*/
			}
			writeToBit(writtenData, writtenDataLengthBytes, writePosBits, reinterpret_cast<const unsigned char*>(data)[i / 8], ofs, blen);
		}
	}

	void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, unsigned char data)
	{
		writeToBit(writtenData, writtenDataLengthBytes, writePosBits, data, 0, 8);
	}

	void BitHelper::writeToBit(void* writtenData, size_t writtenDataLengthBytes, unsigned int* writePosBits, unsigned char data, unsigned int readPosBits, unsigned int readLengthBits)
	{
		unsigned int block = (int)(*writePosBits / 8.0);
		unsigned int offset = (*writePosBits % 8);
		unsigned int notreadlen = 8 - (readLengthBits + readPosBits);

		unsigned char* datas = reinterpret_cast<unsigned char*>(writtenData);

		if (offset > 0)
		{
			datas[block] |= 0xff & ((0xff & (((data >> notreadlen) << notreadlen) << readPosBits)) >> offset);
			if ((8 - offset) < readLengthBits)
			{
				if ((block + 1) >= writtenDataLengthBytes)
				{
					THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The result array is too short.");
				}

				datas[block + 1] |= 0xff & (0xff & ((0xff & ((((data >> notreadlen) << notreadlen) << readPosBits))) << (8 - offset)));
			}
		}
		else
		{
			datas[block] |= (unsigned char)((unsigned char)(((unsigned char)(((data >> notreadlen) << notreadlen) << readPosBits))));
		}

		(*writePosBits) += readLengthBits;
	}
}
