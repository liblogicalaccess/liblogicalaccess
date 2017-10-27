/**
 * \file mifaresmartidreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID reader/card adapter.
 */

#include "mifaresmartidreadercardadapter.hpp"

#include <cstring>

namespace logicalaccess
{
ByteVector MifareSmartIDReaderCardAdapter::sendCommand(unsigned char cmd,
                                                       const ByteVector &command,
                                                       long int timeout)
{
    ByteVector wrappedCommand;

    wrappedCommand.push_back(cmd);
    wrappedCommand.push_back(static_cast<unsigned char>(command.size()));
    wrappedCommand.insert(wrappedCommand.end(), command.begin(), command.end());

    ByteVector result =
        SmartIDReaderCardAdapter::sendCommand(0x80, wrappedCommand, timeout);

    return result;
}

ByteVector MifareSmartIDReaderCardAdapter::request()
{
    ByteVector data;
    data.push_back(static_cast<unsigned char>(0x00));

    return sendCommand(0x41, data);
}

ByteVector MifareSmartIDReaderCardAdapter::anticollision()
{
    ByteVector data;
    data.resize(5, 0x00);

    return sendCommand(0x42, data);
}

void MifareSmartIDReaderCardAdapter::selectIso(const ByteVector &uid)
{
    select(uid);
}

void MifareSmartIDReaderCardAdapter::select(const ByteVector &uid)
{
    sendCommand(0x43, uid);
}

void MifareSmartIDReaderCardAdapter::halt()
{
    sendCommand(0x45, ByteVector());
}
}