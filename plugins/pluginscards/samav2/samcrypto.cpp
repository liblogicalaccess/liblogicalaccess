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

	std::vector<unsigned char> SAMDESfireCrypto::authenticateHostP1(boost::shared_ptr<DESFireKey> key, std::vector<unsigned char> encRndB, unsigned char keyno)
	{
		std::vector<unsigned char> keyvec(key->getData(), key->getData() + key->getLength());

		if (key->getKeyType() == DF_KEY_AES)
			d_cipher.reset(new openssl::AESCipher());
		else if (key->getKeyType() == DF_KEY_3K3DES)
			d_cipher.reset(new openssl::DESCipher());

		openssl::AESSymmetricKey cipherkey = openssl::AESSymmetricKey::createFromData(keyvec);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
		d_rndB.clear();		
		d_cipher->decipher(encRndB, d_rndB, cipherkey, iv, false);
		
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), d_rndB.begin() + 1, d_rndB.begin() + 1 + 15);
		rndB1.push_back(d_rndB[0]);

		RAND_seed(&keyno, sizeof(keyno));

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");

		d_rndA.clear();
		d_rndA.resize(16);
		if (RAND_bytes(&d_rndA[0], static_cast<int>(d_rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> rndAB;
		rndAB.insert(rndAB.end(), d_rndA.begin(), d_rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		std::vector<unsigned char> ret;
		d_cipher->cipher(rndAB, ret, cipherkey, iv, false);
		return ret;
	}

	void SAMDESfireCrypto::authenticateHostP2(unsigned char keyno, std::vector<unsigned char> encRndA1, boost::shared_ptr<DESFireKey> key)
	{
		std::vector<unsigned char> keyvec(key->getData(), key->getData() + key->getLength());
		std::vector<unsigned char> checkRndA;
		std::vector<unsigned char> rndA;
		openssl::AESSymmetricKey cipherkey = openssl::AESSymmetricKey::createFromData(keyvec);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();
		d_cipher->decipher(encRndA1, rndA, cipherkey, iv, false);

		d_sessionKey.clear();
		checkRndA.push_back(rndA[15]);
		checkRndA.insert(checkRndA.end(), rndA.begin(), rndA.begin() + 15);

		if (d_rndA == checkRndA)
		{
			if (key->getKeyType() == DF_KEY_3K3DES)
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 6, d_rndA.begin() + 10);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 6, d_rndB.begin() + 10);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);

				d_cipher.reset(new openssl::DESCipher());
				d_authkey = keyvec;
				d_block_size = 8;
				d_mac_size = 8;
			}
			else
			{
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin(), d_rndA.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin(), d_rndB.begin() + 4);
				d_sessionKey.insert(d_sessionKey.end(), d_rndA.begin() + 12, d_rndA.begin() + 16);
				d_sessionKey.insert(d_sessionKey.end(), d_rndB.begin() + 12, d_rndB.begin() + 16);

				d_cipher.reset(new openssl::AESCipher());
				d_authkey = keyvec;
				d_block_size = 16;
				d_mac_size = 8;
			}

			d_currentKeyNo = keyno;
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authenticateHostP2 Failed!");
	}


	std::vector<unsigned char> SAMDESfireCrypto::sam_aes_encrypt(std::vector<unsigned char> d_sessionKey, std::vector<unsigned char> vectordata)
	{
		std::vector<unsigned char> ret;

		openssl::AESSymmetricKey cipherkey = openssl::AESSymmetricKey::createFromData(d_sessionKey);
		openssl::AESInitializationVector iv = openssl::AESInitializationVector::createNull();


		vectordata.resize(64, 0x00);
		d_cipher->cipher(vectordata, ret, cipherkey, iv, false);
		return ret;
	}
}

