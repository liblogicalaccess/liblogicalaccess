/**
 * \file omnikeyxx21readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 reader unit.
 */

#include "../readers/omnikeyxx21readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../readers/omnikeyxx21readerunitconfiguration.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#if defined(__unix__)
#include <PCSC/reader.h>
#endif

#define CM_IOCTL_GET_SET_RFID_BAUDRATE								SCARD_CTL_CODE(3215)


namespace logicalaccess
{
	OmnikeyXX21ReaderUnit::OmnikeyXX21ReaderUnit(const std::string& name)
		: OmnikeyReaderUnit(name)
	{
		d_readerUnitConfig.reset(new OmnikeyXX21ReaderUnitConfiguration());
	}

	OmnikeyXX21ReaderUnit::~OmnikeyXX21ReaderUnit()
	{
		
	}	

	PCSCReaderUnitType OmnikeyXX21ReaderUnit::getPCSCType() const
	{
		return PCSC_RUT_OMNIKEY_XX21;
	}

	boost::shared_ptr<ReaderCardAdapter> OmnikeyXX21ReaderUnit::getReaderCardAdapter(std::string type)
	{
		return getDefaultReaderCardAdapter();
	}

	void OmnikeyXX21ReaderUnit::changeReaderKey(boost::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key)
	{
		EXCEPTION_ASSERT_WITH_LOG(keystorage, std::invalid_argument, "Key storage must be defined.");
		EXCEPTION_ASSERT_WITH_LOG(key.size() > 0, std::invalid_argument, "key cannot be empty.");

		unsigned char result[256];
		size_t resultlen = 256;

		boost::shared_ptr<PCSCReaderCardAdapter> rca = getDefaultPCSCReaderCardAdapter();
		//rca.reset(new OmnikeyHIDiClassReaderCardAdapter());
		//rca->setReaderUnit(shared_from_this());

		setIsSecureConnectionMode(true);
		//rca->initSecureModeSession(0xFF);
		rca->sendAPDUCommand(0x84, 0x82, (keystorage->getVolatile() ? 0x00 : 0x20), keystorage->getKeySlot(), static_cast<unsigned char>(key.size()), &key[0], key.size(), result, &resultlen);
		//rca->closeSecureModeSession();
		setIsSecureConnectionMode(false);
	}

	void OmnikeyXX21ReaderUnit::getT_CL_ISOType(bool& isTypeA, bool& isTypeB)
	{
		unsigned char outBuffer[64];
		DWORD dwOutBufferSize;
		unsigned char inBuffer[2];
		DWORD dwInBufferSize;
		DWORD dwBytesReturned;
		DWORD dwControlCode = CM_IOCTL_GET_SET_RFID_BAUDRATE;
		DWORD dwStatus;

		memset(outBuffer, 0x00, sizeof(outBuffer));
		dwOutBufferSize = sizeof(outBuffer);
		dwInBufferSize = sizeof(inBuffer);
		dwBytesReturned = 0;

		inBuffer[0] = 0x01;	// Version of command
		inBuffer[1] = 0x00; // Get asymmetric baud rate information

		isTypeA = false;
		isTypeB = false;

		dwStatus = SCardControl(getHandle(), dwControlCode, (LPVOID)inBuffer, dwInBufferSize, (LPVOID)outBuffer, dwOutBufferSize, &dwBytesReturned);
		if (dwStatus == SCARD_S_SUCCESS && dwBytesReturned >= 10)
		{
			switch (outBuffer[9])
			{
			case 0x04:
				isTypeA = true;
				break;

			case 0x08:
				isTypeB = true;
				break;
			}
		}
	}
}
