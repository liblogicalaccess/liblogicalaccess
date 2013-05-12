/**
 * \file datatype.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A Data Type.
 */

#include "logicalaccess/services/accesscontrol/encodings/datatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bcdbytedatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"

#include <cstring>

namespace logicalaccess
{
	DataType::DataType()
		: d_leftParityType(PT_NONE), d_rightParityType(PT_NONE), d_bitDataRepresentationType(ET_BIGENDIAN)
	{

	}

	DataType* DataType::getByEncodingType(EncodingType type)
	{
		switch (type)
		{
		case ET_BCDBYTE:
			return new BCDByteDataType();

		case ET_BCDNIBBLE:
			return new BCDNibbleDataType();
		case ET_BINARY:
			return new BinaryDataType();

		default:
			return NULL;
		}
	}

	long long DataType::pow(int base, unsigned int exp)
	{
		long long ret = 1;
		for (size_t i = 1; i <= exp; i++)
		{
			ret *= base;
		}
		return ret;
	}

	void DataType::setLeftParityType(ParityType type)
	{
		d_leftParityType = type;
	}

	ParityType DataType::getLeftParityType()
	{
		return d_leftParityType;
	}

	void DataType::setRightParityType(ParityType type)
	{
		d_rightParityType = type;
	}

	ParityType DataType::getRightParityType()
	{
		return d_rightParityType;
	}

	void DataType::setBitDataRepresentationType(EncodingType bitDataRepresentationType)
	{
		d_bitDataRepresentationType = bitDataRepresentationType;
	}

	EncodingType DataType::getBitDataRepresentationType()
	{
		return d_bitDataRepresentationType;
	}

	unsigned char DataType::invertBitSex(unsigned char c, size_t length)
	{
		unsigned char ret = 0x00;

		for (size_t i = 0; i < length; i++)
		{
			ret |= (unsigned char)(((c >> i) & 0x01) << (length - i - 1));
		}

		return ret;
	}

	unsigned int DataType::addParityToBuffer(ParityType leftParity, ParityType rightParity, unsigned int blocklen, void* buf, unsigned int buflen, void* procbuf, unsigned int procbuflen)
	{
		if ((buflen % blocklen) != 0 && (leftParity != PT_NONE || rightParity != PT_NONE))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The buffer length is not block length aligned. If you use parity on data type, it must be aligned.");
		}

		unsigned int nbblocks = static_cast<unsigned int>(buflen / blocklen);
		unsigned int newsize = buflen + (nbblocks * (((leftParity != PT_NONE) ? 1 : 0) + ((rightParity != PT_NONE) ? 1 : 0)));

		if (buf != NULL && procbuf != NULL && procbuflen >= newsize)
		{
			unsigned int posproc = 0;
			size_t buflenBytes = (buflen + 7) / 8;
			size_t procbuflenBytes = (procbuflen + 7) / 8;
			unsigned int pos = 0;
			unsigned int currentBlocklen;

			while (pos < buflen)
			{
				if (leftParity != PT_NONE)
				{
					unsigned char parity  = StaticFormat::calculateParity(buf, buflen, leftParity, pos, blocklen);
					BitHelper::writeToBit(procbuf, procbuflenBytes, &posproc, parity, 7, 1);
				}

				currentBlocklen = (pos + blocklen <= buflen) ? blocklen : buflen - pos;
				BitHelper::writeToBit(procbuf, procbuflenBytes, &posproc, buf, buflenBytes, buflen, pos, currentBlocklen);

				if (rightParity != PT_NONE)
				{
					unsigned char parity  = StaticFormat::calculateParity(buf, buflen, rightParity, pos, blocklen);
					BitHelper::writeToBit(procbuf, procbuflenBytes, &posproc, parity, 7, 1);
				}

				pos += currentBlocklen;
			}
		}

		return static_cast<unsigned int>(newsize);
	}

	unsigned int DataType::removeParityToBuffer(ParityType leftParity, ParityType rightParity, unsigned int blocklen, void* buf, unsigned int buflen, void* procbuf, unsigned int procbuflen)
	{
		unsigned int coefParity = (((leftParity != PT_NONE) ? 1 : 0) + ((rightParity != PT_NONE) ? 1 : 0));
		unsigned int nbblocks = static_cast<unsigned int>(buflen / (blocklen + coefParity));
		unsigned int newsize = buflen - (nbblocks * coefParity);

		if ((newsize % blocklen) != 0 && (leftParity != PT_NONE || rightParity != PT_NONE))
		{
			char tmpmsg[64];
			sprintf(tmpmsg, "The buffer length (%d) without parity is not block length aligned. If you use parity on data type, it must be aligned.", static_cast<int>(buflen));
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmpmsg);
		}

		if (buf != NULL && procbuf != NULL && procbuflen >= newsize)
		{
			size_t buflenBytes = (buflen + 7) / 8;

			if (buflen > procbuflen)
			{
				unsigned int posproc = 0;
				size_t procbuflenBytes = (procbuflen + 7) / 8;
				unsigned int currentBlocklen;

				unsigned int pos = 0;
				while (pos < buflen)
				{
					if (leftParity != PT_NONE)
					{
						unsigned char parityToCheck = 0x00;
						unsigned int posparity = 7;
						BitHelper::writeToBit(&parityToCheck, 1, &posparity, buf, buflenBytes, buflen, pos, 1);
						unsigned char parity = StaticFormat::calculateParity(buf, buflen, leftParity, pos++, blocklen);

						if (parityToCheck != parity)
						{
							THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad left parity on DataType.");
						}
					}

					currentBlocklen = ((pos + blocklen +((rightParity != PT_NONE) ? 1 : 0)) <= buflen) ? blocklen : buflen - pos - ((rightParity != PT_NONE) ? 1 : 0);
					BitHelper::writeToBit(procbuf, procbuflenBytes, &posproc, buf, buflenBytes, buflen, pos, currentBlocklen);
					pos += currentBlocklen;

					if (rightParity != PT_NONE)
					{
						unsigned char parityToCheck = 0x00;
						unsigned int posparity = 7;
						BitHelper::writeToBit(&parityToCheck, 1, &posparity, buf, buflenBytes, buflen, pos, 1);
						unsigned char parity = StaticFormat::calculateParity(buf, buflen, rightParity, pos++ - blocklen, blocklen);

						if (parityToCheck != parity)
						{
							THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Bad right parity on DataType.");
						}
					}
				}
			}
			else
			{
				memcpy(procbuf, buf, buflenBytes);
			}
		}

		return static_cast<unsigned int>(newsize);
	}
}

