/**
 * \file iso7816readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 reader/card adapter.
 */

#include "iso7816readercardadapter.hpp"

#include <cstring>

namespace logicalaccess
{
void ISO7816ReaderCardAdapter::sendAPDUCommand(const ByteVector &command,
                                               unsigned char *result, size_t *resultlen)
{
    ByteVector res = sendCommand(command);

    if (result != nullptr && resultlen != nullptr)
    {
        if (*resultlen >= res.size())
        {
            *resultlen = res.size();
            memcpy(result, &res[0], res.size());
        }
    }
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     unsigned char lc,
                                                     const ByteVector &data,
                                                     unsigned char le)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(lc);
    command.insert(command.end(), data.begin(), data.end());
    command.push_back(le);

    return sendCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     unsigned char lc,
                                                     const ByteVector &data)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(lc);
    command.insert(command.end(), data.begin(), data.end());

    return sendCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     const ByteVector &data)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.insert(command.end(), data.begin(), data.end());

    return sendCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     unsigned char le)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(le);

    return sendCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     unsigned char lc, unsigned char le)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(lc);
    command.push_back(le);

    return sendCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
                                                     unsigned char p1, unsigned char p2)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);

    return sendCommand(command);
}
}