/**
 * \file desfirecrypto.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire cryptographic functions.
 */

#include "samcrypto.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <zlib.h>
#include <ctime>
#include <cstdlib>

#ifndef UNIX
#include "logicalaccess/msliblogicalaccess.h"
#endif

#include <cstring>

#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"


namespace logicalaccess
{		
	SAMDESfireCrypto::SAMDESfireCrypto()
	{
	}

	SAMDESfireCrypto::~SAMDESfireCrypto()
	{
		
	}

	std::vector<unsigned char> SAMDESfireCrypto::authenticate_PICC1(unsigned char keyno, unsigned char *diversify, const std::vector<unsigned char>& encRndB)
	{
		/*d_sessionKey.clear();
		d_authkey.resize(16);
		d_profile->getKey(d_currentAid, keyno, diversify, d_authkey);
		d_rndB = desfire_CBC_send(d_authkey, std::vector<unsigned char>(), encRndB);
		
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 8);
		rndB1.push_back(d_rndB[0]);

		RAND_seed(&keyno, sizeof(keyno));

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		d_rndA.clear();
		d_rndA.resize(8);
		if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		return sam_CBC_send(d_authkey, std::vector<unsigned char>(), rndAB);*/
		return std::vector<unsigned char>();
	}

	void SAMDESfireCrypto::authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
	{
		/*std::vector<unsigned char> rndA = sam_CBC_send(d_authkey, std::vector<unsigned char>(), encRndA1);
		std::vector<unsigned char> checkRndA;

		d_sessionKey.clear();
		checkRndA.push_back(rndA[7]);
		checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 7);

		if (d_rndA == checkRndA)
		{
			// DES
			if (!memcmp(&d_authkey[0], &d_authkey[8], 8))
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
			}
			// 3DES
			else
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 4, d_rndA.begin() + 8);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 4, d_rndB.begin() + 8);
			}

			d_currentKeyNo = keyno;
		}

		d_auth_method = CM_LEGACY;
		d_mac_size = 4;*/
	}
}

