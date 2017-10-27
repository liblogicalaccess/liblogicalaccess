/**
 * \file omnikeyxx21readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 reader unit.
 */

#include "../readers/omnikeylanxx21readerunit.hpp"
#include "../pcscreaderprovider.hpp"

#include "logicalaccess/myexception.hpp"
#include <thread>

#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(__unix__)
#include <PCSC/reader.h>
#elif defined(__APPLE__)

#ifndef SCARD_CTL_CODE
#define SCARD_CTL_CODE(code) (0x42000000 + (code))
#endif

#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#endif

#define CM_IOCTL_READER_CONNECT SCARD_CTL_CODE(3410)
#define CM_IOCTL_READER_DISCONNECT SCARD_CTL_CODE(3411)

namespace logicalaccess
{
/*
 * Implementation based on omnikey "OMNIKEY 5121 Ethernet Encoder_Advanced_ User
 * guide_L001437W_ Rev 1.0" documentation
 */
OmnikeyLANXX21ReaderUnit::OmnikeyLANXX21ReaderUnit(const std::string &name)
    : OmnikeyXX21ReaderUnit(name)
{
}

OmnikeyLANXX21ReaderUnit::~OmnikeyLANXX21ReaderUnit()
{
}

PCSCReaderUnitType OmnikeyLANXX21ReaderUnit::getPCSCType() const
{
    return PCSC_RUT_OMNIKEY_LAN_XX21;
}

bool OmnikeyLANXX21ReaderUnit::connectToReader()
{
    LOG(LogLevel::INFOS) << "Connecting to LAN reader...";
    // Connecting to LAN reader before using standard PC/SC commands
    sendControlMode(CM_IOCTL_READER_CONNECT);
    LOG(LogLevel::INFOS) << "Connected successfully to the LAN reader. Now connecting to "
                            "PC/SC reader with standard method...";

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return OmnikeyXX21ReaderUnit::connectToReader();
}

void OmnikeyLANXX21ReaderUnit::disconnectFromReader()
{
    LOG(LogLevel::INFOS) << "Disconnecting from PC/SC reader with standard method...";
    OmnikeyXX21ReaderUnit::disconnectFromReader();

    LOG(LogLevel::INFOS) << "Disconnecting from LAN reader...";
    sendControlMode(CM_IOCTL_READER_DISCONNECT);
    LOG(LogLevel::INFOS) << "Disconnected successfully from the LAN reader.";

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void OmnikeyLANXX21ReaderUnit::sendControlMode(DWORD dwControlCode) const
{
    LOG(LogLevel::INFOS) << "Sending LAN control code {" << dwControlCode
                         << "} on device {" << d_name << "}...";

    unsigned char outBuffer[512];
    unsigned char inBuffer[512];

    memset(inBuffer, 0x00, sizeof(inBuffer));
    memset(outBuffer, 0x00, sizeof(outBuffer));
    DWORD dwInBufferSize    = sizeof(inBuffer);
    DWORD dwOutBufferSize   = sizeof(outBuffer);
    DWORD dwBytesReturned   = 0;
    DWORD dwAP              = 0;
    SCARDHANDLE hCardHandle = 0;

    // Direct connect to Omnikey LAN reader
    DWORD dwStatus = SCardConnect(getPCSCReaderProvider()->getContext(),
                                  reinterpret_cast<const char *>(d_name.c_str()),
                                  SCARD_SHARE_DIRECT, 0, &hCardHandle, &dwAP);
    if (SCARD_S_SUCCESS != dwStatus)
    {
        LOG(LogLevel::ERRORS) << "Error {" << std::hex << dwStatus << std::dec << "}";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "LAN SCardConnect returned an error.");
    }

    // Use the direct connection to send the Connect control code.
    *((unsigned long *)inBuffer) = 6000; // Recommended Timeout 6 sec.
    dwStatus = SCardControl(hCardHandle, dwControlCode, inBuffer, dwInBufferSize,
                            outBuffer, dwOutBufferSize, &dwBytesReturned);

    if (SCARD_S_SUCCESS != dwStatus)
    {
        LOG(LogLevel::ERRORS) << "Error {" << std::hex << dwStatus << "} - {"
                              << *(ULONG *)outBuffer << std::dec << "}";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "LAN SCardControl returned an error.");
    }

    // Disconnect from direct mode
    dwStatus = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
    if (SCARD_S_SUCCESS != dwStatus)
    {
        LOG(LogLevel::ERRORS) << "Error {" << std::hex << dwStatus << std::dec << "}";
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "LAN SCardDisconnect returned an error.");
    }
}
}
