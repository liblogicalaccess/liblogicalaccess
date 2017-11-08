/**
 * \file iso7816iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

#include <cstring>

namespace logicalaccess
{
ISO7816ISO7816Commands::ISO7816ISO7816Commands()
    : ISO7816Commands(CMD_ISO7816ISO7816)
{
}

ISO7816ISO7816Commands::ISO7816ISO7816Commands(std::string ct)
    : ISO7816Commands(ct)
{
}

ISO7816ISO7816Commands::~ISO7816ISO7816Commands()
{
}

void ISO7816ISO7816Commands::setP1P2(size_t offset, short efid, unsigned char &p1,
                                     unsigned char &p2)
{
    p1 = 0x00;
    p2 = 0x00;

    if (efid == 0)
    {
        p1 = 0x7f & (offset >> 8);
    }
    else
    {
        p1 = 0x80 | (0x0f & efid);
    }
    p2 = 0xff & offset;
}

ByteVector ISO7816ISO7816Commands::readBinary(size_t length, size_t offset, short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        0x00, 0xB0, p1, p2, static_cast<unsigned char>(length));

    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::writeBinary(const ByteVector &data, size_t offset,
                                         short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        0x00, 0xD0, p1, p2, static_cast<unsigned char>(data.size()), data);
}

void ISO7816ISO7816Commands::updateBinary(const ByteVector &data, size_t offset,
                                          short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        0x00, 0xD6, p1, p2, static_cast<unsigned char>(data.size()), data);
}

void ISO7816ISO7816Commands::eraseBinary(size_t offset, short efid)
{
    unsigned char p1, p2;

    setP1P2(offset, efid, p1, p2);
    getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0x0E, p1, p2);
}

ByteVector ISO7816ISO7816Commands::getData(size_t /*length*/, unsigned short dataObject)
{
    ByteVector result = getISO7816ReaderCardAdapter()->sendAPDUCommand(
        0x00, 0xCA, (0xff & (dataObject >> 8)), (0xff & dataObject));
    return ByteVector(result.begin(), result.end() - 2);
}

void ISO7816ISO7816Commands::putData(const ByteVector &data, unsigned short dataObject)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(
        0x00, 0xDA, (0xff & (dataObject >> 8)), (0xff & dataObject),
        static_cast<unsigned char>(data.size()), data);
}

void ISO7816ISO7816Commands::selectFile(unsigned short efid)
{
    unsigned char p1 = 0x00;
    unsigned char p2 = 0x00;
    unsigned char data[2];
    data[0] = 0xff & (efid >> 8);
    data[1] = 0xff & efid;

    selectFile(p1, p2, data, sizeof(data));
}

void ISO7816ISO7816Commands::selectFile(unsigned char *dfname, size_t dfnamelen)
{
    unsigned char p1 = 0x04;
    unsigned char p2 = 0x00;

    selectFile(p1, p2, dfname, dfnamelen);
}

void ISO7816ISO7816Commands::selectFile(unsigned char p1, unsigned char p2,
                                        unsigned char *data, size_t datalen)
{
    getISO7816ReaderCardAdapter()->sendAPDUCommand(0x00, 0xA4, p1, p2,
                                                   static_cast<unsigned char>(datalen),
                                                   ByteVector(data, data + datalen));
}
}