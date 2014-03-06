/**
 * \file SAMAV1ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav1iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav1chip.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"
#include "samcommands.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/cmac.hpp"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/thread/thread_time.hpp>
#include <cstring>

namespace logicalaccess
{
	SAMAV1ISO7816Commands::SAMAV1ISO7816Commands()
	{

		d_named_mutex.reset(new boost::interprocess::named_mutex(boost::interprocess::open_or_create, "sam_mutex"));
		bool locked = d_named_mutex->timed_lock(boost::get_system_time() + boost::posix_time::seconds(5));

		boost::interprocess::shared_memory_object shm_obj(boost::interprocess::open_or_create, "sam_memory", boost::interprocess::read_write);
		boost::interprocess::offset_t size = 0;
		shm_obj.get_size(size);
		if (size != 4)
			shm_obj.truncate(4);

		d_region.reset(new boost::interprocess::mapped_region(shm_obj, boost::interprocess::read_write));

		char *addr = (char*)d_region->get_address();

		if (size != 4 || !locked)
			std::memset(addr, 0, d_region->get_size());

		unsigned char x = 0;
		for (; x < d_region->get_size() && addr[x] != 0; ++x);

		if (x < d_region->get_size())
		{
			addr[x] = 1;
			d_cla = DEFAULT_SAM_CLA + x;
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "No channel available.");

		d_named_mutex->unlock();
	}

	SAMAV1ISO7816Commands::~SAMAV1ISO7816Commands()
	{
		d_named_mutex->lock();
		char *addr = (char*)d_region->get_address();
		addr[d_cla - 0x80] = 0;
		d_named_mutex->unlock();

		if (!boost::interprocess::shared_memory_object::remove("sam_memory"))
			INFO_SIMPLE_("SAM Shared Memory removed failed. It is probably still open by a process.");

		//we do not remove named_mutex - it can still be used by another process
	}

	SAMVersion SAMAV1ISO7816Commands::getVersion()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		SAMVersion	info;
		memset(&info, 0x00, sizeof(SAMVersion));

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x60, 0x00, 0x00, 0x00, result, &resultlen);


		if (resultlen == 33 && result[31] == 0x90 && result[32] == 0x00)
		{
			memcpy(&info, result, sizeof(info));
			
			/*if (info.hardware.vendorid == 0x04)
				std::cout << "Vendor: NXP" << std::endl;
			if (info.hardware.majorversion == 0x03)
				std::cout << "Major version: T1AD2060" << std::endl;
			if (info.hardware.minorversion == 0x04)
				std::cout << "Major version: T1AR1070" << std::endl;
			std::cout << "Storage size: " << (unsigned int)info.hardware.storagesize << std::endl;
			std::cout << "Communication protocol type : " << (unsigned int)info.hardware.protocoltype << std::endl;*/
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getVersion failed.");

		return info;
	}

	boost::shared_ptr<SAMKeyEntry>	SAMAV1ISO7816Commands::getKeyEntry(unsigned char keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		boost::shared_ptr<SAMKeyEntry> keyentry;
		KeyEntryInformation keyentryinformation;

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x64, keyno, 0x00, 0x00, result, &resultlen);

		if ((resultlen == 14 || resultlen == 13) &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
		{
			keyentry.reset(new SAMKeyEntry());

			memcpy(keyentryinformation.set, result + resultlen - 4, 2);
			keyentry->setSET(keyentryinformation.set);

			keyentryinformation.kuc = result[resultlen - 5];
			keyentryinformation.cekv = result[resultlen - 6];
			keyentryinformation.cekno = result[resultlen - 7];
			keyentryinformation.desfirekeyno =  result[resultlen - 8];

			memcpy(keyentryinformation.desfireAid, result + resultlen - 11, 3);


			if (resultlen == 13)
			{
				keyentryinformation.verb = result[resultlen - 12];
				keyentryinformation.vera = result[resultlen - 13];
			}
			else
			{
				keyentryinformation.verc = result[resultlen - 12];
				keyentryinformation.verb = result[resultlen - 13];
				keyentryinformation.vera = result[resultlen - 14];
			}

			keyentry->setKeyEntryInformation(keyentryinformation);
			keyentry->setKeyTypeFromSET();
			keyentry->setUpdateMask(0);
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKeyEntry failed.");
		return keyentry;
	}

	void SAMAV1ISO7816Commands::changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry> keyentry, boost::shared_ptr<DESFireKey> key)
	{
		if (d_crypto->d_sessionKey.size() == 0)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char proMas = 0;
		proMas = keyentry->getUpdateMask();

		size_t buffer_size = keyentry->getLength() + sizeof(KeyEntryInformation);
		unsigned char *data = new unsigned char[buffer_size];
		memset(data, 0, buffer_size);

		memcpy(data, &*(keyentry->getData()), keyentry->getLength());
		memcpy(data + 48, &keyentry->getKeyEntryInformation(), sizeof(KeyEntryInformation));
		
		std::vector<unsigned char> iv;
		iv.resize(16, 0x00);

		std::vector<unsigned char> vectordata(data, data + buffer_size);
		
		std::vector<unsigned char> encdatalittle;
		
		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xc1, keyno, proMas, (unsigned char)(encdatalittle.size()), &encdatalittle[0], encdatalittle.size(), result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKeyEntry failed.");
	}


	void SAMAV1ISO7816Commands::activeAV2Mode(boost::shared_ptr<Key> masterKey, unsigned char masterKeyVersion)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char p1_part1 = 0x03;

		std::vector<unsigned char> maxChainBlocks(3, 0x00);

		std::vector<unsigned char> data_p1(2, 0x00);
		data_p1[1] = masterKeyVersion;

		data_p1.insert(data_p1.end(), maxChainBlocks.begin(), maxChainBlocks.end());

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x10, p1_part1, 0x00, 0x05, &data_p1[0],  0x05, 0x00, result, &resultlen);
		if (resultlen == 14 && result[12] == 0x90 && (result[13] == 0x00 || result[13] == 0xAF))
		{
			std::vector<unsigned char> keycipher(masterKey->getData(), masterKey->getData() + masterKey->getLength());
			boost::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher(new openssl::AESCipher());
			std::vector<unsigned char> emptyIV(16), rnd1;

			std::vector<unsigned char>  diversify(result, result + 12); //Rnd2
			diversify.push_back(p1_part1); //P1_part1

			diversify.insert(diversify.end(), maxChainBlocks.begin(), maxChainBlocks.end()); //MaxChainBlocks - unlimited

			std::vector<unsigned char> macHost = openssl::CMACCrypto::cmac(keycipher, d_cipher, 16, diversify, emptyIV, 16);

			RAND_seed(result, 12);
			EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
			rnd1.resize(12);
			if (RAND_bytes(&rnd1[0], static_cast<int>(rnd1.size())) != 1)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
			}

			std::vector<unsigned char> data_p2;
			data_p2.insert(data_p2.end(), macHost.begin(), macHost.begin() + 8);
			data_p2.insert(data_p2.end(), rnd1.begin(), rnd1.end());

			resultlen = sizeof(result);
			getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x10, 0x00, 0x00, 0x14, &data_p2[0], 0x14, 0x00, result, &resultlen);
			if (resultlen == 26 && result[24] == 0x90 && result[25] == 0x00)
			{

			}
		}
	}

	void SAMAV1ISO7816Commands::authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		if (key->getKeyType() == DF_KEY_DES)
			authentificateHostDES(key, keyno);
		else
			authentificateHost_AES_3K3DES(key, keyno);
	}

	void SAMAV1ISO7816Commands::authentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char authMode = 0x00;
		unsigned char data[2];

		memset(data, 0, sizeof(data));
		data[0] = keyno;
		data[1] = key->getKeyVersion();


		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, authMode, 0x00, 0x02, data, 0x02, 0x00, result, &resultlen);
		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0xaf))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost_AES_3K3DES P1 failed.");

		std::vector<unsigned char> encRndB(result, result + resultlen - 2);
		std::vector<unsigned char> encRndAB = d_crypto->authenticateHostP1(key, encRndB, keyno);

		memset(result, 0, sizeof(result));
		resultlen = sizeof(result);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), &encRndAB[0], encRndAB.size(), 0x00, result, &resultlen);
		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHost_AES_3K3DES P2 failed.");

		std::vector<unsigned char> encRndA1(result, result + resultlen - 2);
		d_crypto->authenticateHostP2(keyno, encRndA1, key);
	}

	void SAMAV1ISO7816Commands::authentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char authMode = 0x00;
		unsigned char data[2];
		size_t keylength = key->getLength();

		memset(data, 0, sizeof(data));
		data[0] = keyno;
		data[1] = key->getKeyVersion();


		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, authMode, 0x00, 0x02, data, 0x02, 0x00, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0xaf))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES P1 failed.");

		std::vector<unsigned char> keyvec(key->getData(), key->getData() + keylength);
		
		std::vector<unsigned char> iv(keylength);
		memset(&iv[0], 0, keylength);

		//get encRNB
		std::vector<unsigned char> encRNB(result, result + resultlen - 2);

		//dec RNB
		std::vector<unsigned char> RndB =  d_crypto->desfire_CBC_send(keyvec, iv, encRNB);

		//Create RNB'
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), RndB.begin() + 1, RndB.begin() + RndB.size());
		rndB1.push_back(RndB[0]);

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		//Create our RndA
		std::vector<unsigned char>  rndA(8);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot retrieve cryptographically strong bytes");
		}

		//create rndAB
		std::vector<unsigned char> rndAB;
		rndAB.clear();
		rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());

		//enc rndAB
		std::vector<unsigned char> encRndAB =  d_crypto->desfire_CBC_send(keyvec, iv, rndAB);

		//send enc rndAB
		resultlen = sizeof(result);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), &encRndAB[0], encRndAB.size(), 0x00, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES P2 failed.");


		std::vector<unsigned char> encRndA1(result, result + resultlen - 2);
		std::vector<unsigned char> dencRndA1 =  d_crypto->desfire_CBC_send(keyvec, iv, encRndA1);

		//create rndA'
		std::vector<unsigned char> rndA1;
		rndA1.insert(rndA1.end(), rndA.begin() + 1, rndA.begin() + rndA.size());
		rndA1.push_back(rndA[0]);
		
		//Check if RNDA is our
		if (!std::equal(dencRndA1.begin(), dencRndA1.end(), rndA1.begin()))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "authentificateHostDES Final Check failed.");

		d_crypto->d_sessionKey.clear();

		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);
	}

	std::string SAMAV1ISO7816Commands::getSAMTypeFromSAM()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x60, 0x00, 0x00, 0x00, result, &resultlen);

		if (resultlen > 3)
		{
			if (result[resultlen - 3] == 0xA1)
				return "SAM_AV1";
			else if (result[resultlen - 3] == 0xA2)
				return "SAM_AV2";
		}
		return "SAM_NONE";
	}

	 boost::shared_ptr<SAMKucEntry> SAMAV1ISO7816Commands::getKUCEntry(unsigned char kucno)
	 {
		boost::shared_ptr<SAMKucEntry> kucentry(new SAMKucEntry);
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x6c, kucno, 0x00, 0x00, result, &resultlen);

		if (resultlen == 12 && (result[resultlen - 2] == 0x90 || result[resultlen - 1] == 0x00))
		{
			SAMKUCEntryStruct kucentrys;
			memcpy(&kucentrys, result, sizeof(SAMKUCEntryStruct));
			kucentry->setKucEntryStruct(kucentrys);
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "getKUCEntry failed.");
		return kucentry;
	 }
	 
	 void SAMAV1ISO7816Commands::changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> kucentry, boost::shared_ptr<DESFireKey> key)
	 {
		if (d_crypto->d_sessionKey.size() == 0)
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: AuthentificationHost have to be done before use such command.");

		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char data[6] = {};
		memcpy(data, &kucentry->getKucEntryStruct(), 6);
		std::vector<unsigned char> vectordata(data, data + 6);
		std::vector<unsigned char> encdatalittle;

		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		int proMas = kucentry->getUpdateMask();

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xcc, kucno, proMas, 0x08, &encdatalittle[0], encdatalittle.size(), result, &resultlen);

		if (resultlen >= 2 && (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "changeKUCEntry failed.");
	 }

	 void SAMAV1ISO7816Commands::disableKeyEntry(unsigned char keyno) 
	 {
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xd8, keyno, 0x00, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "disableKeyEntry failed.");
	 }

	 void SAMAV1ISO7816Commands::selectApplication(unsigned char *aid)
	 {
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0x5a, 0x00, 0x00, 0x03, aid, 0x03, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "selectApplication failed.");
	 }

	 std::vector<unsigned char> SAMAV1ISO7816Commands::dumpSessionKey()
	 {
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xd5, 0x00, 0x00, 0x00, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "dumpSessionKey failed.");

		return std::vector<unsigned char>(result, result + resultlen - 2);
	 }


	 std::vector<unsigned char> SAMAV1ISO7816Commands::decipherData(std::vector<unsigned char> data, bool islastdata)
	 {
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char p1 = 0x00;
		std::vector<unsigned char> datawithlength(3);

		if (!islastdata)
			p1 = 0xaf;
		else
		{
			datawithlength[0] = (unsigned char)(data.size() & 0xff0000);
			datawithlength[1] = (unsigned char)(data.size() & 0x00ff00);
			datawithlength[2] = (unsigned char)(data.size() & 0x0000ff);
		}
		datawithlength.insert(datawithlength.end(), data.begin(), data.end());

		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xdd, p1, 0x00, (unsigned char)(datawithlength.size()), &datawithlength[0], datawithlength.size(), 0x00, result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] != 0x90 &&
			((p1 == 0x00 && result[resultlen - 1] != 0x00) || (p1 == 0xaf && result[resultlen - 1] != 0xaf)))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "decipherData failed.");

		return std::vector<unsigned char>(result, result + resultlen - 2);
	 }

	 std::vector<unsigned char> SAMAV1ISO7816Commands::encipherData(std::vector<unsigned char> data, bool islastdata)
	 {
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		unsigned char p1 = 0x00;

		if (!islastdata)
			p1 = 0xaf;
		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xed, p1, 0x00, (unsigned char)(data.size()), &data[0], data.size(), 0x00, result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] != 0x90 &&
			((p1 == 0x00 && result[resultlen - 1] != 0x00) || (p1 == 0xaf && result[resultlen - 1] != 0xaf)))
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "encipherData failed.");

		return std::vector<unsigned char>(result, result + resultlen - 2);
	 }


	 std::vector<unsigned char> SAMAV1ISO7816Commands::changeKeyPICC(const ChangeKeyInfo& info)
	 {
		//if (d_crypto->d_sessionKey.size() == 0)
		//	THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Failed: Authentification have to be done before using such command.");

		unsigned char keyCompMeth = 0;
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		if (!info.oldKeyInvolvement)
			keyCompMeth = 1;

		unsigned char cfg = info.desfireNumber & 0xf;
		if (info.isMasterKey)
			cfg |= 0x10;
		std::vector<unsigned char> data(4);
		data[0] = info.currentKeyNo;
		data[1] = info.currentKeyV;
		data[2] = info.newKeyNo;
		data[3] = info.newKeyV;
		getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xc4, keyCompMeth, cfg, (unsigned char)(data.size()), &data[0], data.size(), 0x00, result, &resultlen);

		if (resultlen >= 2 &&  (result[resultlen - 2] != 0x90 || result[resultlen - 1] != 0x00))
		{
			char tmp[64];
			sprintf(tmp, "changeKeyPICC failed (%x %x).", result[resultlen - 2], result[resultlen - 1]);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmp);
		}

		return std::vector<unsigned char>(result, result + resultlen - 2);
	 }
}
