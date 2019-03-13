/**
 * \file iso7816readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 reader/card adapter.
 */

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816response.hpp>

#include <cstring>

namespace logicalaccess
{
ISO7816Response ISO7816ReaderCardAdapter::sendAPDUCommand(const ByteVector &data)
{
    return ISO7816Response(sendCommand(data));
}

ISO7816Response ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
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

    return sendAPDUCommand(command);
}

ISO7816Response
ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
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

    return sendAPDUCommand(command);
}

ISO7816Response ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla,
                                                          unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     const ByteVector &data)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.insert(command.end(), data.begin(), data.end());

    return sendAPDUCommand(command);
}

ISO7816Response ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla,
                                                          unsigned char ins,
                                                     unsigned char p1, unsigned char p2,
                                                     unsigned char le)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(le);

    return sendAPDUCommand(command);
}

ISO7816Response
ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla, unsigned char ins,
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

    return sendAPDUCommand(command);
}

ISO7816Response ISO7816ReaderCardAdapter::sendAPDUCommand(unsigned char cla,
                                                          unsigned char ins,
                                                     unsigned char p1, unsigned char p2)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);

    return sendAPDUCommand(command);
}

ISO7816Response ISO7816ReaderCardAdapter::sendExtendedAPDUCommand(
    unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2,
    unsigned short lc, const ByteVector &data)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(0x00);
    command.push_back(static_cast<unsigned char>((lc >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>(lc & 0xff));
    command.insert(command.end(), data.begin(), data.end());

    return sendAPDUCommand(command);
}

ISO7816Response ISO7816ReaderCardAdapter::sendExtendedAPDUCommand(
    unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2,
    unsigned short lc, const ByteVector &data, unsigned short le)
{
    ByteVector command;
    command.push_back(cla);
    command.push_back(ins);
    command.push_back(p1);
    command.push_back(p2);
    command.push_back(0x00);
    command.push_back(static_cast<unsigned char>((lc >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>(lc & 0xff));
    command.insert(command.end(), data.begin(), data.end());
    command.push_back(static_cast<unsigned char>((le >> 8) & 0xff));
    command.push_back(static_cast<unsigned char>(le & 0xff));

    return sendAPDUCommand(command);
}

ByteVector ISO7816ReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    ByteVector r;
    if (crypto_ && crypto_->secureMode())
        r = crypto_->decrypt_rapdu(answer);
    else
        r = answer;
    return r;
}

ByteVector ISO7816ReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    if (crypto_ && crypto_->secureMode())
        return crypto_->encrypt_apdu(command);
    return command;
}

void ISO7816ReaderCardAdapter::setCrypto(std::shared_ptr<ISO24727Crypto> crypto)
{
    crypto_ = crypto;
}
}