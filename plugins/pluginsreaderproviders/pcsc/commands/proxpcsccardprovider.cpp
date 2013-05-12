/**
 * \file proxpcsccardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox PC/SC card profiles.
 */

#include "../commands/proxpcsccardprovider.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "proxlocation.hpp"

#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"

namespace logicalaccess
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
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
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