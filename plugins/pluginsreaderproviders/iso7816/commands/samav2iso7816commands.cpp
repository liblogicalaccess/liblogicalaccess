/**
 * \file desfireiso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire ISO7816 commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav2iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav2chip.hpp"
#include "samcrypto.hpp"
#include "samav2keyentry.hpp"
#include "samav2kucentry.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"

#include <cstring>

namespace logicalaccess
{
	SAMAV2ISO7816Commands::SAMAV2ISO7816Commands()
	{
	}

	SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
	{
	}

	void		SAMAV2ISO7816Commands::GetVersion()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		SAMVersion	info;

		std::cout << "GetVersion Commands Called" << std::endl;

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x60, 0x00, 0x00, 0x00, result, &resultlen);


		if (resultlen == 33 && result[31] == 0x90 && result[32] == 0x00)
		{
			memcpy(&info, result, sizeof(info));
			
			if (info.hardware.vendorid == 0x04)
				std::cout << "Vendor: NXP" << std::endl;
			if (info.hardware.majorversion == 0x03)
				std::cout << "Major version: T1AD2060" << std::endl;
			if (info.hardware.minorversion == 0x04)
				std::cout << "Major version: T1AR1070" << std::endl;
			std::cout << "Storage size: " << (unsigned int)info.hardware.storagesize << std::endl;
			std::cout << "Communication protocol type : " << (unsigned int)info.hardware.protocoltype << std::endl;
		}
		else
			std::cout << "GetVersion has failed" << std::endl;
		std::cout << "Called Done" << std::endl;
	}

	boost::shared_ptr<SAMAV2KeyEntry>	SAMAV2ISO7816Commands::GetKeyEntry(unsigned int keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		boost::shared_ptr<SAMAV2KeyEntry> keyentry;
		boost::shared_ptr<KeyEntryInformation> keyentryinformation(new KeyEntryInformation);

		std::cout << "GetKeyEntry Commands Called" << std::endl;

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x64, keyno, 0x00, 0x00, result, &resultlen);

		if ((resultlen == 14 || resultlen == 13) &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
		{
			keyentry.reset(new SAMAV2KeyEntry());
			unsigned short *set;

			set = reinterpret_cast<unsigned short*>(result + resultlen - 4);
			memcpy(keyentryinformation->set, result + resultlen - 4, 2);
			keyentry->setSET(keyentryinformation->set);

			keyentryinformation->kuc = result[resultlen - 5];
			keyentryinformation->cekv = result[resultlen - 6];
			keyentryinformation->cekno = result[resultlen - 7];
			keyentryinformation->desfirekeyno =  result[resultlen - 8];

			memcpy(keyentryinformation->desfireAid, result + resultlen - 11, 3);


			if (resultlen == 13)
			{
				keyentryinformation->verb = result[resultlen - 12];
				keyentryinformation->vera = result[resultlen - 13];
			}
			else
			{
				keyentryinformation->verc = result[resultlen - 12];
				keyentryinformation->verb = result[resultlen - 13];
				keyentryinformation->vera = result[resultlen - 14];
			}

			keyentry->setKeyEntryInformation(keyentryinformation);
			keyentry->setKeyTypeFromSET();
			keyentry->setUpdateMask(0);

			std::cout << "GetKeyEntry SUCCED" << std::endl;
		}
		else
			std::cout << "GetKeyEntry FAILED" << std::endl;
		std::cout << "Called Done" << std::endl;
		return keyentry;
	}

	void		SAMAV2ISO7816Commands::ChangeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> keyentry, boost::shared_ptr<DESFireKey> key)
	{
		std::cout << "ChangeKeyEntry Commands Called" << std::endl;
		if (d_crypto->d_sessionKey.size() == 0)
		{
			std::cout << "Failed: AuthentificationHost have to be done before use such command." << std::endl;
			return;
		}
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char proMas = 0;

		//proMas = 0x81;

		proMas = keyentry->getUpdateMask();

		size_t buffer_size = keyentry->getLength() + sizeof(KeyEntryInformation);
		unsigned char *data = new unsigned char[buffer_size];
		memset(data, 0, buffer_size);

		memcpy(data, &*(keyentry->getData()), keyentry->getLength());
		memcpy(data + 48, &*(keyentry->getKeyEntryInformation()), sizeof(KeyEntryInformation));
		
	//	data[15] = 1;
	//	data[57] = 1;
		//data[54] = 0xff;
		std::vector<unsigned char> iv;
		iv.resize(16, 0x00);

		std::vector<unsigned char> vectordata(data, data + buffer_size);
		
		std::vector<unsigned char> encdatalittle;
		
		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xc1, keyno, proMas, (unsigned char)(encdatalittle.size()), &encdatalittle[0], encdatalittle.size(), result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "SUCCED !!" << std::endl;
		else
			std::cout << "Failed :(" << std::endl;

		std::cout << "Called Done" << std::endl;
	}


	void		SAMAV2ISO7816Commands::ActiveAV2Mode()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		std::cout << std::endl << "ActiveAV2Mode Commands Called" << std::endl;

		unsigned char data[5];
		memset(data, 0, sizeof(data));
		unsigned char p1 = 3;
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x10, p1, 0x00, 0x05, data,  0x05, 0x00, result, &resultlen);

		std::cout << resultlen << std::endl;

		
		std::cout << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;

		std::cout << "Called Done" << std::endl;
	}

	void			SAMAV2ISO7816Commands::AuthentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		std::cout << std::endl << "AuthentificateHost Commands Called" << std::endl;

		if (key->getKeyType() == DF_KEY_DES)
			AuthentificateHostDES(key, keyno);
		else
			AuthentificateHost_AES_3K3DES(key, keyno);

		std::cout << "Called Done" << std::endl;
	}

	void SAMAV2ISO7816Commands::AuthentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		std::cout << std::endl << "AuthentificateHostAES Commands Called" << std::endl;
		unsigned char authMode = 0x00;
		unsigned char data[2];
		size_t keylength = key->getLength();

		memset(data, 0, sizeof(data));
		data[0] = keyno;
		data[1] = key->getKeyVersion();


		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, authMode, 0x00, 0x02, data, 0x02, 0x00, result, &resultlen);
		std::cout << "P1 enRndB + result: " << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;
		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0xaf)
			std::cout << "P1 Command to SAM SUCCED !!" << std::endl;
		else
		{
			std::cout << "P1 Failed :(" << std::endl;
			return;
		}

		std::vector<unsigned char> encRndB(result, result + resultlen - 2);

		std::vector<unsigned char> encRndAB = d_crypto->authenticateHostP1(key, encRndB, keyno);

		memset(result, 0, sizeof(result));
		resultlen = sizeof(result);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), &encRndAB[0], encRndAB.size(), 0x00, result, &resultlen);
		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "P2 Command to SAM SUCCED !!" << std::endl;
		else
		{
			std::cout << "P2 Failed :(" << std::endl;
			return;
		}

		std::vector<unsigned char> encRndA1(result, result + resultlen - 2);
		d_crypto->authenticateHostP2(keyno, encRndA1, key);


		std::cout << "Called Done" << std::endl;
	}

	void SAMAV2ISO7816Commands::AuthentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno)
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);
		std::cout << std::endl << "AuthentificateHostDES Commands Called" << std::endl;
		unsigned char authMode = 0x00;
		unsigned char data[2];
		size_t keylength = key->getLength();

		memset(data, 0, sizeof(data));
		data[0] = keyno;
		data[1] = key->getKeyVersion();


		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, authMode, 0x00, 0x02, data, 0x02, 0x00, result, &resultlen);

		std::cout << "P1 enRndB + result: " << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;
		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0xaf)
			std::cout << "P1 Command to SAM SUCCED !!" << std::endl;
		else
		{
			std::cout << "P1 Failed :(" << std::endl;
			return;
		}

		std::vector<unsigned char> keyvec(key->getData(), key->getData() + keylength);
		
		std::vector<unsigned char> iv(keylength);
		memset(&iv[0], 0, keylength);

		//get encRNB
		std::vector<unsigned char> encRNB(result, result + resultlen - 2);

		//dec RNB
		std::vector<unsigned char> RndB =  d_crypto->desfire_CBC_send(keyvec, iv, encRNB);
		std::cout << "P1 RndB: " << BufferHelper::getHex(RndB) << std::endl;

		//Create RNB'
		std::vector<unsigned char> rndB1;
		rndB1.insert(rndB1.end(), RndB.begin() + 1, RndB.begin() + RndB.size());
		rndB1.push_back(RndB[0]);
		std::cout << "P1 RndB': " << BufferHelper::getHex(rndB1) << std::endl;

		EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException, "Insufficient enthropy source");
		//Create our RndA
		std::vector<unsigned char>  rndA(8);
		if (RAND_bytes(&rndA[0], static_cast<int>(rndA.size())) != 1)
		{
			throw LibLogicalAccessException("Cannot retrieve cryptographically strong bytes");
		}

		//create rndAB
		std::vector<unsigned char> rndAB;
		rndAB.clear();
		rndAB.insert(rndAB.end(), rndA.begin(), rndA.end());
		rndAB.insert(rndAB.end(), rndB1.begin(), rndB1.end());
		std::cout << "P1 RndAB: " << BufferHelper::getHex(rndAB) << std::endl;

		//enc rndAB
		std::vector<unsigned char> encRndAB =  d_crypto->desfire_CBC_send(keyvec, iv, rndAB);

		//send enc rndAB
		resultlen = sizeof(result);
		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xa4, 0x00, 0x00, (unsigned char)(encRndAB.size()), &encRndAB[0], encRndAB.size(), 0x00, result, &resultlen);

		std::cout << "P2 encRndA: " << BufferHelper::getHex(std::vector<unsigned char>(result, result + resultlen)) << std::endl;
		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "P2 Command to SAM SUCCED !!" << std::endl;
		else
		{
			std::cout << "P2 Failed :(" << std::endl;
			return;
		}


		std::vector<unsigned char> encRndA1(result, result + resultlen - 2);
		std::vector<unsigned char> dencRndA1 =  d_crypto->desfire_CBC_send(keyvec, iv, encRndA1);

		//create rndA'
		std::vector<unsigned char> rndA1;
		rndA1.insert(rndA1.end(), rndA.begin() + 1, rndA.begin() + rndA.size());
		rndA1.push_back(rndA[0]);
		
		std::cout << "P2 dencRndA': " << BufferHelper::getHex(dencRndA1) << std::endl;
		std::cout << "P2 rndA': " << BufferHelper::getHex(rndA1) << std::endl;


		//Check if RNDA is our
		if (std::equal(dencRndA1.begin(), dencRndA1.end(), rndA1.begin()))
			std::cout << "AuthentificationHost SUCCES !" << std::endl;
		else
		{
			std::cout << "AuthentificationHost Failed :( !" << std::endl;
			return;
		}

		d_crypto->d_sessionKey.clear();

		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), rndA.begin(), rndA.begin() + 4);
		d_crypto->d_sessionKey.insert(d_crypto->d_sessionKey.end(), RndB.begin(), RndB.begin() + 4);

		std::cout << "Called Done" << std::endl;
	}

	std::string			SAMAV2ISO7816Commands::GetSAMTypeFromSAM()
	{
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x60, 0x00, 0x00, 0x00, result, &resultlen);

		if (resultlen > 3)
		{
			if (result[resultlen - 3] == 0xA1)
			{
				std::cout << "GetVersion say that the SAM is AV1" << std::endl;
				return "SAM_AV1";
			}
			else if (result[resultlen - 3] == 0xA2)
			{
				std::cout << "GetVersion say that the SAM is AV2" << std::endl;
				return "SAM_AV2";
			}
		}
		return "SAM_NONE";
	}

	 boost::shared_ptr<SAMAV2KucEntry>		SAMAV2ISO7816Commands::GetKUCEntry(unsigned int keyno)
	 {
		std::cout << std::endl << "GetKUCEntry Commands Called" << std::endl;
		boost::shared_ptr<SAMAV2KucEntry> kucentry(new SAMAV2KucEntry);
		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x6c, keyno, 0x00, 0x00, result, &resultlen);

		if (resultlen == 12 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
		{
			std::cout << "SUCCED !!" << std::endl;
			memcpy(&*(kucentry->getKucEntryStruct()), result, sizeof(SAMAV2KUCEntryStruct));
		}
		else
		{
			std::cout << "Failed :(" << std::endl;
		}

		std::cout << "Called Done" << std::endl;
		return kucentry;
	 }
	 
	 void									SAMAV2ISO7816Commands::ChangeKUCEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KucEntry> keyentry, boost::shared_ptr<DESFireKey> key)
	 {
		std::cout << std::endl << "ChangeKUCEntry Commands Called" << std::endl;
		if (d_crypto->d_sessionKey.size() == 0)
		{
			std::cout << "Failed: AuthentificationHost have to be done before use such command." << std::endl;
			return;
		}


		unsigned char result[255];
		size_t resultlen = sizeof(result);

		unsigned char data[6] = {};

		memcpy(data, &*(keyentry->getKucEntryStruct()), 6);

		std::vector<unsigned char> iv;
		iv.resize(16, 0x00);

		std::vector<unsigned char> vectordata(data, data + 6);
		
		std::vector<unsigned char> encdatalittle;

		if (key->getKeyType() == DF_KEY_DES)
			encdatalittle = d_crypto->sam_encrypt(d_crypto->d_sessionKey, vectordata);
		else
			encdatalittle = d_crypto->sam_crc_encrypt(d_crypto->d_sessionKey, vectordata, key);

		int proMas = keyentry->getUpdateMask();

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xcc, keyno, proMas, 0x08, &encdatalittle[0], encdatalittle.size(), result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "SUCCED !!" << std::endl;
		else
			std::cout << "Failed :(" << std::endl;
		 std::cout << "Called Done" << std::endl;
	 }

	 void									SAMAV2ISO7816Commands::DisableKeyEntry(unsigned char keyno) 
	 {
		std::cout << std::endl << "DisableKeyEntry Commands Called" << std::endl;

		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0xd8, keyno, 0x00, result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "SUCCED !!" << std::endl;
		else
			std::cout << "Failed :(" << std::endl;

		 std::cout << "Called Done" << std::endl;
	 }

	 void									SAMAV2ISO7816Commands::SelectApplication(unsigned char *aid)
	 {
		 std::cout << std::endl << "SelectApplication Commands Called" << std::endl;

		unsigned char result[255];
		size_t resultlen = sizeof(result);

		getISO7816ReaderCardAdapter()->sendAPDUCommand(0x80, 0x5a, 0x00, 0x00, 0x03, aid, 0x03, result, &resultlen);

		if (resultlen >= 2 &&  result[resultlen - 2] == 0x90 && result[resultlen - 1] == 0x00)
			std::cout << "SUCCED !!" << std::endl;
		else
			std::cout << "Failed :(" << std::endl;

		 std::cout << "Called Done" << std::endl;
	 }
}
