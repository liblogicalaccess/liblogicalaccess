/**
 * \file AccessInfo.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Base class access informations. 
 */
 
#include "logicalaccess/Cards/AccessInfo.h"
#include "logicalaccess/Services/AccessControl/Formats/Format.h"
#include <time.h>
#include <stdlib.h>

#include "logicalaccess/logs.h"

#include <openssl/rand.h>

#include <sstream>
#include <iomanip>

namespace LOGICALACCESS
{
	AccessInfo::AccessInfo()
	{
	}

	string AccessInfo::generateSimpleKey(long seed, size_t keySize)
	{
		std::ostringstream oss;
		RAND_seed(&seed, sizeof(seed));

#ifdef _WINDOWS
		RAND_screen();
#endif

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLOGICALACCESSException, "Insufficient enthropy source");

		std::vector<unsigned char> buf;
		buf.resize(keySize, 0x00);

		if (RAND_bytes(&buf[0], static_cast<int>(keySize)) != 1)
		{
			throw LibLOGICALACCESSException("Cannot retrieve cryptographically strong bytes");
		}
		
		for (size_t i = 0; i < buf.size(); ++i)
		{
			if (i != 0)
			{
				oss << " ";
			}

			oss << std::hex << std::setfill('0') << std::setw(2) << (size_t)buf[i];
		}

		return oss.str();
	}

	string AccessInfo::generateSimpleDESKey(long seed, size_t keySize)
	{
		std::ostringstream oss;
		RAND_seed(&seed, sizeof(seed));

#ifdef _WINDOWS
		RAND_screen();
#endif

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLOGICALACCESSException, "Insufficient enthropy source");

		std::vector<unsigned char> buf;
		buf.resize(keySize, 0x00);

		if (RAND_bytes(&buf[0], static_cast<int>(keySize)) != 1)
		{
			throw LibLOGICALACCESSException("Cannot retrieve cryptographically strong bytes");
		}
		
		for (size_t i = 0; i < buf.size(); ++i)
		{
			if (i != 0)
			{
				oss << " ";
			}

			buf.data()[i] = buf[i] & (0xfe | Format::calculateParity(&buf[i], 1, PT_ODD, (size_t)0, 7));

			oss << std::hex << std::setfill('0') << std::setw(2) << (size_t)buf[i];
		}

		return oss.str();
	}

	bool AccessInfo::operator==(const AccessInfo& ai) const
	{
		return (typeid(ai) == typeid(*this));
	}
}

