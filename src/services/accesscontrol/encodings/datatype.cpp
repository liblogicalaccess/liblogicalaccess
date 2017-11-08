/**
 * \file datatype.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A Data Type.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/encodings/datatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bcdbytedatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

#include <cstring>

namespace logicalaccess
{
DataType::DataType()
    : d_leftParityType(PT_NONE)
    , d_rightParityType(PT_NONE)
    , d_bitDataRepresentationType(ET_BIGENDIAN)
{
}

DataType *DataType::getByEncodingType(EncodingType type)
{
    switch (type)
    {
    case ET_BCDBYTE: return new BCDByteDataType();

    case ET_BCDNIBBLE: return new BCDNibbleDataType();
    case ET_BINARY: return new BinaryDataType();

    default: return nullptr;
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

ParityType DataType::getLeftParityType() const
{
    return d_leftParityType;
}

void DataType::setRightParityType(ParityType type)
{
    d_rightParityType = type;
}

ParityType DataType::getRightParityType() const
{
    return d_rightParityType;
}

void DataType::setBitDataRepresentationType(EncodingType bitDataRepresentationType)
{
    d_bitDataRepresentationType = bitDataRepresentationType;
}

EncodingType DataType::getBitDataRepresentationType() const
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

BitsetStream DataType::addParity(ParityType leftParity, ParityType rightParity,
                                 unsigned int blocklen, BitsetStream &buf)
{
    if ((buf.getBitSize() % blocklen) != 0 &&
        (leftParity != PT_NONE || rightParity != PT_NONE))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The buffer length is not block length aligned. If you "
                                 "use parity on data type, it must be aligned.");
    }

    BitsetStream procbuf;

    unsigned int pos = 0;

    while (pos < buf.getBitSize())
    {
        if (leftParity != PT_NONE)
        {
            unsigned char parity =
                StaticFormat::calculateParity(buf, leftParity, pos, blocklen);
            procbuf.append(parity, 7, 1);
        }

        unsigned int currentBlocklen =
            (pos + blocklen <= buf.getBitSize()) ? blocklen : buf.getBitSize() - pos;
        procbuf.concat(buf.getData(), pos, currentBlocklen);

        if (rightParity != PT_NONE)
        {
            unsigned char parity =
                StaticFormat::calculateParity(buf, rightParity, pos, blocklen);
            procbuf.append(parity, 7, 1);
        }

        pos += currentBlocklen;
    }

    return procbuf;
}

BitsetStream DataType::removeParity(ParityType leftParity, ParityType rightParity,
                                    unsigned int blocklen, BitsetStream &buf)
{
    BitsetStream procbuf;

    unsigned int pos = 0;
    while (pos < buf.getBitSize())
    {
        if (leftParity != PT_NONE)
        {
            BitsetStream parityToCheck(1 * 8);
            unsigned int posparity = 7;
            parityToCheck.writeAt(posparity, buf.getData(), pos, 1);
            unsigned char parity =
                StaticFormat::calculateParity(buf, leftParity, pos, blocklen);

            if (parityToCheck.getData()[0] != parity)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "Bad left parity on DataType.");
            }

            ++pos;
        }

        const unsigned int currentBlocklen =
            ((pos + blocklen) <= buf.getBitSize()) ? blocklen : buf.getBitSize() - pos;
        procbuf.concat(buf.getData(), pos, currentBlocklen);
        pos += currentBlocklen;

        if (rightParity != PT_NONE)
        {
            BitsetStream parityToCheck(1 * 8);
            unsigned int posparity = 7;
            parityToCheck.writeAt(posparity, buf.getData(), pos, 1);
            unsigned char parity =
                StaticFormat::calculateParity(buf, rightParity, pos - blocklen, blocklen);

            if (parityToCheck.getData()[0] != parity)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "Bad right parity on DataType.");
            }

            ++pos;
        }
    }

    return procbuf;
}

unsigned char DataType::getBitDataSize() const
{
    unsigned char ret = 0;

    switch (getType())
    {
    case ET_BCDBYTE: ret      = 8; break;
    case ET_BCDNIBBLE: ret    = 4; break;
    case ET_BINARY: ret       = 8; break;
    case ET_BIGENDIAN: ret    = 8; break;
    case ET_LITTLEENDIAN: ret = 8; break;
    case ET_NOENCODING: break;
    default:;
    }
    return ret;
}
}