/**
 * \file ProxPCSCCardProvider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox PC/SC card profiles.
 */

#include "../Commands/ProxPCSCCardProvider.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::endl;
using std::dec;
using std::hex;
using std::setfill;
using std::setw;
using std::ostringstream;
using std::istringstream;

#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "ProxLocation.h"

#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"

namespace LOGICALACCESS
{
	ProxPCSCCardProvider::ProxPCSCCardProvider()
		: ProxCardProvider()
	{

	}

	ProxPCSCCardProvider::~ProxPCSCCardProvider()
	{

	}

	unsigned int ProxPCSCCardProvider::readData(boost::shared_ptr<ProxLocation> location, void* data, size_t dataLength, unsigned int dataLengthBits)
	{
		unsigned int readbits = 0;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		if (!location)
		{
			location.reset(new ProxLocation());
			location->bit = 0;
		}

		if (d_cardAdapter)
		{
			unsigned char atr[64];
			size_t atrLength = d_cardAdapter->getPCSCReaderUnit()->getATR(atr, sizeof(atr));

			if (data != NULL && ((dataLengthBits + (location->bit + 7)) / 8) <= atrLength)
			{
				unsigned int writePosBit = 0;
				BitHelper::writeToBit(data, dataLength, &writePosBit, atr, atrLength, static_cast<unsigned int>(atrLength * 8), location->bit, dataLengthBits);
				readbits = dataLengthBits;
			}
			else
			{
				readbits = static_cast<unsigned int>(atrLength * 8);
			}
		}

		return readbits;
	}
}