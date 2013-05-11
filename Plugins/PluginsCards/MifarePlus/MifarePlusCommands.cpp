/**
 * \file MifarePlusCommands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Commands for MifarePlus card.
 */

#include "MifarePlusCommands.h"

#include <stdlib.h>


namespace logicalaccess
{
	MifarePlusCommands::MifarePlusCommands()
	{
		d_crypto = boost::shared_ptr<MifarePlusCrypto>(new MifarePlusCrypto());
	}

	MifarePlusCommands::~MifarePlusCommands()
	{
	}

	std::vector<unsigned char> MifarePlusCommands::LeftRotateSB(std::vector<unsigned char> string)
	{	
		unsigned char retain;
		size_t size = string.size();
		size_t c = 1;

		retain = string[0];
		while (size > 1 &&  c < size)
		{
			string[c - 1] = string[c];
			c++;
		}
		string[c - 1] = retain;

		return (string);
	}

	std::vector<unsigned char> MifarePlusCommands::GetRandKey(const void* init, size_t size)
	{
		std::vector<unsigned char> rand;

		if (init != NULL)
			RAND_seed(init, sizeof(init));
		
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLOGICALACCESSException, "Insufficient enthropy source");
		
		rand.clear();
		rand.resize(size);
		if (RAND_bytes(&rand[0], static_cast<int>(rand.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Cannot retrieve cryptographically strong bytes");
		}

		return (rand);
	}

	boost::shared_ptr<MifarePlusCrypto> MifarePlusCommands::GetCrypto()
	{
		return (d_crypto);
	}
}