/**
 * \file omnikeyxx21readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 reader unit.
 */

#include "../readers/omnikeylanxx21readerunit.hpp"
#include "../pcscreaderprovider.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(__unix__)
#include <PCSC/reader.h>
#endif

#define CM_IOCTL_READER_CONNECT				SCARD_CTL_CODE (3410) 
#define CM_IOCTL_READER_DISCONNECT			SCARD_CTL_CODE (3411) 

namespace logicalaccess
{
	/*
	 * Implementation based on omnikey "OMNIKEY 5121 Ethernet Encoder_Advanced_ User guide_L001437W_ Rev 1.0" documentation
	 */
	OmnikeyLANXX21ReaderUnit::OmnikeyLANXX21ReaderUnit(const std::string& name)
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
		INFO_("Connecting to LAN reader...");
		// Connecting to LAN reader before using standard PC/SC commands
		sendControlMode(CM_IOCTL_READER_CONNECT);
		INFO_("Connected successfully to the LAN reader. Now connecting to PC/SC reader with standard method...");

#ifdef _WINDOWS
		Sleep(500);
#elif defined(__unix__)
		usleep(500000);
#endif

		return OmnikeyXX21ReaderUnit::connectToReader();
	}

	void OmnikeyLANXX21ReaderUnit::disconnectFromReader()
	{
		INFO_("Disconnecting from PC/SC reader with standard method...");
		OmnikeyXX21ReaderUnit::disconnectFromReader();

		INFO_("Disconnecting from LAN reader...");
		sendControlMode(CM_IOCTL_READER_DISCONNECT);
		INFO_("Disconnected successfully from the LAN reader.");

#ifdef _WINDOWS
		Sleep(500);
#elif defined(__unix__)
		usleep(500000);
#endif
	}

	void OmnikeyLANXX21ReaderUnit::sendControlMode(DWORD dwControlCode)
	{
		INFO_("Sending LAN control code {%d} on device {%s}...", dwControlCode, d_name.c_str());

		SCARDHANDLE hCardHandle;
		unsigned char outBuffer[512];
		DWORD dwOutBufferSize;
		unsigned char inBuffer[512];
		DWORD dwInBufferSize;
		DWORD dwBytesReturned;
		DWORD dwAP;
		DWORD dwStatus;

		memset(inBuffer, 0x00, sizeof(inBuffer));
		memset(outBuffer, 0x00, sizeof(outBuffer));
		dwInBufferSize = sizeof(inBuffer);
		dwOutBufferSize = sizeof(outBuffer);
		dwBytesReturned = 0;
		dwAP = 0;
		hCardHandle = 0;

		// Direct connect to Omnikey LAN reader
		dwStatus = SCardConnect(getPCSCReaderProvider()->getContext(), reinterpret_cast<const char*>(d_name.c_str()), SCARD_SHARE_DIRECT, 0, &hCardHandle, &dwAP);
		if (SCARD_S_SUCCESS != dwStatus)
		{
			ERROR_("Error {%x}", dwStatus);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "LAN SCardConnect returned an error.");
		}

		// Use the direct connection to send the Connect control code.
		*((unsigned long*)inBuffer) = 6000; // Recommended Timeout 6 sec.
		dwStatus = SCardControl(hCardHandle, dwControlCode, inBuffer, dwInBufferSize, outBuffer, dwOutBufferSize, &dwBytesReturned);

		if (SCARD_S_SUCCESS != dwStatus) 
		{
			ERROR_("Error {%x} - {%x}", dwStatus, *(ULONG *)outBuffer);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "LAN SCardControl returned an error.");
		}

		// Disconnect from direct mode 
		dwStatus = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD); 
		if (SCARD_S_SUCCESS != dwStatus) 
		{
			ERROR_("Error {%x}", dwStatus);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "LAN SCardDisconnect returned an error.");
		}
	}
}
