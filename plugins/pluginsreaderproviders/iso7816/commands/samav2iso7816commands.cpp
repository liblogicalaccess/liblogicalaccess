/**
 * \file SAMAV2ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav2iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/cmac.hpp"

#include <cstring>

namespace logicalaccess
{
	SAMAV2ISO7816Commands::SAMAV2ISO7816Commands() : d_cmdCtr(0)
	{
	}

	SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
	{
	}

	void SAMAV2ISO7816Commands::generateSessionKey(std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2)
	{
		std::vector<unsigned char> SV1a(16), SV2a(16), SV1b(16), emptyIV(16);

		std::copy(rnd1.begin() + 11, rnd1.begin() + 16, SV1a.begin());
		std::copy(rnd2.begin() + 11, rnd2.begin() + 16, SV1a.begin() + 5);
		std::copy(rnd1.begin() + 4, rnd1.begin() + 9, SV1a.begin() + 10);

		for (unsigned char x = 4; x <= 9; ++x)
		{
			SV1a[x + 6] ^= rnd2[x];
		}

		std::copy(rnd1.begin() + 7, rnd1.begin() + 12, SV2a.begin());
		std::copy(rnd2.begin() + 7, rnd2.begin() + 12, SV2a.begin() + 5);
		std::copy(rnd1.begin(), rnd1.begin() + 5, SV2a.begin() + 10);

		for (unsigned char x = 0; x <= 4; ++x)
		{
			SV2a[x + 10] ^= rnd2[x];
		}

		SV1a[15] = 0x91; /* AES 128 */
		SV2a[15] = 0x82; /* AES 128 */
		/* TODO AES 192 */

		boost::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_authKey)));
		boost::shared_ptr<openssl::InitializationVector> iv(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(emptyIV)));
		boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());

		cipher->cipher(SV1a, d_sessionKey, *symkey.get(), *iv.get(), false);

		iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(d_sessionKey)));
		cipher->cipher(SV2a, d_macSessionKey, *symkey.get(), *iv.get(), false);
	}

	void SAMAV2ISO7816Commands::authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		std::vector<unsigned char> result;
		std::vector<unsigned char> data_p1(3, 0x00);
		data_p1[0] = keyno;
		data_p1[1] = key->getKeyVersion();
		data_p1[2] = 2; //Host Mode: Full Protection


		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x03, data_p1, 0x00);
		if (result.size() != 14 || result[12] != 0x90 || result[13] != 0xAF)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost P1 Failed.");

		std::vector<unsigned char> keycipher(key->getData(), key->getData() + key->getLength());
		boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher(new openssl::AESCipher());
		std::vector<unsigned char> emptyIV(16), rnd1;

		/* Create rnd2 for p3 - CMAC: rnd2 | Host Mode | ZeroPad */
		std::vector<unsigned char>  rnd2(result.begin(), result.begin() + 12);
		rnd2.push_back(2); //Host Mode: Full Protection
		rnd2.resize(16); //ZeroPad


		std::vector<unsigned char> macHost = openssl::CMACCrypto::cmac(keycipher, cipher, 16, rnd2, emptyIV, 16);
		truncateMacBuffer(macHost);

		RAND_seed(&result[0], 12);
		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		rnd1.resize(12);
		if (RAND_bytes(&rnd1[0], static_cast<int>(rnd1.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		std::vector<unsigned char> data_p2;
		data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + 8);
		data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x14, data_p2, 0x00);
		if (result.size() != 26 || result[24] != 0x90 || result[25] != 0xAF)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost P2 Failed.");

		/* Check CMAC - Create rnd1 for p3 - CMAC: rnd1 | P1 | other data */
		rnd1.insert(rnd1.end(), rnd2.begin() + 12, rnd2.end()); //p2 data without rnd2

		macHost = openssl::CMACCrypto::cmac(keycipher, cipher, 16, rnd1, emptyIV, 16);
		truncateMacBuffer(macHost);

		for (unsigned char x = 0; x < 8; ++x)
		{
			if (macHost[x] != result[x])
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost P2 CMAC from SAM is Wrong.");
		}

		/* Create kxe - d_authKey */
		generateAuthEncKey(keycipher, rnd1, rnd2);


		//create rndA
		std::vector<unsigned char> rndA(16);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}


		//decipher rndB
		boost::shared_ptr<openssl::SymmetricKey> symkey(new openssl::AESSymmetricKey(openssl::AESSymmetricKey::createFromData(d_authKey)));
		boost::shared_ptr<openssl::InitializationVector> iv(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(emptyIV)));

		std::vector<unsigned char> encRndB(result.begin() + 8, result.end() - 2);
		std::vector<unsigned char> dencRndB;

		cipher->decipher(encRndB, dencRndB, *symkey.get(), *iv.get(), false);

		//create rndB'
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.begin(), dencRndB.begin() + 2, dencRndB.begin() + dencRndB.size());
		rndB1.push_back(dencRndB[0]);
		rndB1.push_back(dencRndB[1]);

		std::vector<unsigned char> dataHost, encHost;
		dataHost.insert(dataHost.end(), rndA.begin(), rndA.end()); //RndA
		dataHost.insert(dataHost.end(), rndB1.begin(), rndB1.end()); //RndB'

		iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(emptyIV)));
		cipher->cipher(dataHost, encHost, *symkey.get(), *iv.get(), false);

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, 0x20, encHost, 0x00);
		if (result.size() != 18 || result[16] != 0x90 || result[17] != 0x00)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost P3 Failed.");

		std::vector<unsigned char> encSAMrndA(result.begin(), result.end() - 2), SAMrndA;
		iv.reset(new openssl::AESInitializationVector(openssl::AESInitializationVector::createFromData(emptyIV)));
		cipher->decipher(encSAMrndA, SAMrndA, *symkey.get(), *iv.get(), false);
		SAMrndA.insert(SAMrndA.begin(), SAMrndA.end() - 2, SAMrndA.end());

		if (!std::equal(SAMrndA.begin(), SAMrndA.begin() + 16, rndA.begin()))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost P3 RndA from SAM is invalide.");

		generateSessionKey(rnd1, rnd2);
		d_cmdCtr = 0;
	}

	typedef struct  s_KeyEntryAV2Information
	{
		unsigned char desfireAid[3];
		unsigned char desfirekeyno;
		unsigned char cekno;
		unsigned char cekv;
		unsigned char kuc;
		unsigned char set[2];
		unsigned char vera;
		unsigned char verb;
		unsigned char verc;
		unsigned char ExtSet;
	}				KeyEntryAV2Information;

	boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > SAMAV2ISO7816Commands::getKeyEntry(unsigned char keyno)
	{
		std::vector<unsigned char> result;
		boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry;
		KeyEntryAV2Information keyentryinformation;

		result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x64, keyno, 0x00, 0x00);
		if ((result.size() == 15 || result.size() == 14) &&  result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
		{
			keyentry.reset(new SAMKeyEntry<KeyEntryAV2Information, SETAV2>(SAMType::SAM_AV2));
			keyentryinformation.ExtSet = result[result.size() - 3];
			memcpy(keyentryinformation.set, &result[result.size() - 5], 2);
			keyentry->setSET(keyentryinformation.set);

			keyentryinformation.kuc = result[result.size() - 6];
			keyentryinformation.cekv = result[result.size() - 7];
			keyentryinformation.cekno = result[result.size() - 8];
			keyentryinformation.desfirekeyno =  result[result.size() - 9];

			memcpy(keyentryinformation.desfireAid, &result[result.size() - 12], 3);


			if (result.size() == 13)
			{
				keyentryinformation.verb = result[result.size() - 13];
				keyentryinformation.vera = result[result.size() - 14];
			}
			else
			{
				keyentryinformation.verc = result[result.size() - 13];
				keyentryinformation.verb = result[result.size() - 14];
				keyentryinformation.vera = result[result.size() - 15];
			}

			keyentry->setKeyEntryInformation(keyentryinformation);
			keyentry->setKeyTypeFromSET();
			keyentry->setUpdateMask(0);
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
		return 	boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> >();
	}

	boost::shared_ptr<SAMKucEntry> SAMAV2ISO7816Commands::getKUCEntry(unsigned char kucno)
	{
				return 	boost::shared_ptr<SAMKucEntry>();
	}

	void SAMAV2ISO7816Commands::changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key)
	{
	}

	void SAMAV2ISO7816Commands::changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, boost::shared_ptr<DESFireKey> key)
	{
	}
}
