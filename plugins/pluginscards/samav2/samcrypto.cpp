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
		return DESFireCrypto::authenticate_PICC1(keyno, diversify, encRndB);
	}

	void SAMDESfireCrypto::authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1)
	{
		DESFireCrypto::authenticate_PICC2(keyno, encRndA1);
	}
}

