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
        : StaticFormat(), d_leftParityLength(0x00), d_leftParityType(PT_NONE), d_rightParityLength(0x00), d_rightParityType(PT_NONE)
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
        //unsigned char* data = new unsigned char[dataLength];
        //memset(data, 0x00, dataLength);
		BitsetStream data;

        data.concat(getLinearDataWithoutParity());
        unsigned char ret = calculateParity(data, d_leftParityType, 1, d_leftParityLength);

        return ret;
    }

    unsigned char WiegandFormat::getRightParity() const
    {
        size_t dataLength = (getDataLength() + 7) / 8;
        //unsigned char* data = new unsigned char[dataLength];
        //memset(data, 0x00, dataLength);
		BitsetStream data;

        data.concat(getLinearDataWithoutParity());
        unsigned char ret = calculateParity(data, d_rightParityType, getDataLength() - d_rightParityLength - 1, d_rightParityLength);

        return ret;
    }

    std::vector<uint8_t> WiegandFormat::getLinearData() const
    {
		BitsetStream data;

		data.concat(getLinearDataWithoutParity());
		//unsigned int pos = 0;

        if (d_leftParityType != PT_NONE)
        {
            //BitHelper::writeToBit(data, dataLengthBytes, &pos, getLeftParity(), 7, 1);
			data.writeAt(0, getLeftParity(), 7, 1);
        }

        //pos = getDataLength() - 1;

        if (d_rightParityType != PT_NONE)
        {
            //BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity(), 7, 1);
			data.writeAt(getDataLength() - 1, getRightParity(), 7, 1);
        }
		return data.getData();
    }

    void WiegandFormat::setLinearData(const std::vector<uint8_t>& data)
    {
        if (data.size() * 8 < getDataLength())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
        }

        setLinearDataWithoutParity(data);

        unsigned int pos = 0;
        int par;
        if (d_leftParityType != PT_NONE)
        {
            par = getLeftParity();
            if (data[pos / 8] >> 7 != par)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Left parity format error.");
            }
        }

        pos = getDataLength() - 1;

        if (d_rightParityType != PT_NONE)
        {
            par = getRightParity();
            if (data[pos / 8] << (pos % 8) >> 7 != par)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Left parity format error.");
            }
        }
    }
}