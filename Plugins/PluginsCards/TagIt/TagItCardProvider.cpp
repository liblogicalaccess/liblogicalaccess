/**
 * \file TagItCardProvider.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Tag-It card profiles.
 */

#include "TagItCardProvider.h"


namespace LOGICALACCESS
{
	TagItCardProvider::TagItCardProvider()
	{

	}

	TagItCardProvider::~TagItCardProvider()
	{

	}

	bool TagItCardProvider::isLocked(size_t block)
	{
		unsigned char status = getISO15693Commands()->getSecurityStatus(block);
		
		return ((status & 0x01) != 0);
	}
}