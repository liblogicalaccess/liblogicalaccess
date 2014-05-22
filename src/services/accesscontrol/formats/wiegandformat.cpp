/**
 * \file wiegandformat.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand Format Base.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegandformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"

#include <cstring>

namespace logicalaccess
{
	WiegandFormat::WiegandFormat()
		: StaticFormat(), d_rightParityLength(0x00), d_leftParityLength(0x00)
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());
	}

	WiegandFormat::~WiegandFormat()
	{
		
	}

	unsigned char WiegandFormat::getLeftParity() const
	{
		//getDataLength is in bits => we have to convert in bytes to allocate data
		size_t dataLength = (getDataLength() + 7) / 8;
		unsigned char* data = new unsigned char[dataLength];
		memset(data, 0x00, dataLength);

		getLinearDataWithoutParity(data, dataLength);
		unsigned char ret = calculateParity(data, dataLength, d_leftParityType, 1, d_leftParityLength);

		delete[] data;
		return ret;
	}

	unsigned char WiegandFormat::getRightParity() const
	{
		size_t dataLength = (getDataLength() + 7) / 8;
		unsigned char* data = new unsigned char[dataLength];
		memset(data, 0x00, dataLength);

		getLinearDataWithoutParity(data, dataLength);
		unsigned char ret = calculateParity(data, dataLength, d_rightParityType, getDataLength() - d_rightParityLength - 1, d_rightParityLength);

		delete[] data;
		return ret;
	}

	void WiegandFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		getLinearDataWithoutParity(data, dataLengthBytes);
		unsigned int pos = 0;

		if (d_leftParityType != PT_NONE)
		{
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getLeftParity(), 7, 1);
		}

		pos = getDataLength() - 1;

		if (d_rightParityType != PT_NONE)
		{
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity(), 7, 1);
		}
	}

	void WiegandFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
		if (dataLengthBytes*8 < getDataLength())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
		}

		setLinearDataWithoutParity(data, dataLengthBytes);

		unsigned int pos = 0;
		int par;
		if (d_leftParityType != PT_NONE)
		{
			par = getLeftParity();
			if ((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] >> 7) != par)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Left parity format error.");
			}
		}

		pos = getDataLength() - 1;

		if (d_rightParityType != PT_NONE)
		{
			par = getRightParity();
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != par)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Left parity format error.");
				return;
			}
		}
	}
}

