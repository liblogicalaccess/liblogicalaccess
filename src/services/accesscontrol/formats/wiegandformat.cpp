/**
 * \file wiegandformat.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand Format Base.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegandformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>

#include <cstring>

namespace logicalaccess
{
WiegandFormat::WiegandFormat()
    : StaticFormat()
    , d_leftParityLength(0x00)
    , d_leftParityType(PT_NONE)
    , d_rightParityLength(0x00)
    , d_rightParityType(PT_NONE)
{
    d_dataType.reset(new BinaryDataType());
    d_dataRepresentation.reset(new BigEndianDataRepresentation());
}

WiegandFormat::~WiegandFormat()
{
}

unsigned char WiegandFormat::getLeftParity() const
{
    BitsetStream data;

    data.concat(getLinearDataWithoutParity());
    unsigned char ret = calculateParity(data, d_leftParityType, 1, d_leftParityLength);

    return ret;
}

unsigned char WiegandFormat::getRightParity() const
{
    BitsetStream data;

    data.concat(getLinearDataWithoutParity());
    unsigned char ret =
        calculateParity(data, d_rightParityType,
                        getDataLength() - d_rightParityLength - 1, d_rightParityLength);

    return ret;
}

ByteVector WiegandFormat::getLinearData() const
{
    BitsetStream data;

    data.concat(getLinearDataWithoutParity());

    if (d_leftParityType != PT_NONE)
    {
        data.writeAt(0, getLeftParity(), 7, 1);
    }

    if (d_rightParityType != PT_NONE)
    {
        data.writeAt(getDataLength() - 1, getRightParity(), 7, 1);
    }
    return data.getData();
}

void WiegandFormat::setLinearData(const ByteVector &data)
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
        if ((unsigned char)(data[pos / 8] >> 7) != par)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Left parity format error.");
        }
    }

    pos = getDataLength() - 1;

    if (d_rightParityType != PT_NONE)
    {
        par = getRightParity();
        if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) != par)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Left parity format error.");
        }
    }
}
}