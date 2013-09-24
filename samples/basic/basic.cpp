/**
 * \file main.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "pcscreaderunit.hpp"
#include "pcscreaderunitconfiguration.hpp"
#include "iso7816readerunit.hpp"
#include "iso7816readerunitconfiguration.hpp"
#include "commands/samav1iso7816commands.hpp"
#include "commands/desfireiso7816commands.hpp"
#include "desfirechip.hpp"
#include "samchip.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "nxpkeydiversification.hpp"
#include "sagemkeydiversification.hpp"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

/**
 * \brief The application entry point.
 * \param argc The arguments count.
 * \param argv The arguments.
 */
int main(int , char**)
{
	try
	{
		std::vector<std::string> readerList = logicalaccess::LibraryManager::getInstance()->getAvailableReaders();
		std::cout << "Available reader plug-ins ("<< readerList.size() <<"):" << std::endl;
		for (std::vector<std::string>::iterator it = readerList.begin(); it != readerList.end(); ++it)
		{
			std::cout << "\t" << (*it) << std::endl;
		}

		std::vector<std::string> cardList = logicalaccess::LibraryManager::getInstance()->getAvailableCards();
		std::cout << "Available card plug-ins ("<< cardList.size() <<"):" << std::endl;
		for (std::vector<std::string>::iterator it = cardList.begin(); it != cardList.end(); ++it)
		{
			std::cout << "\t" << (*it) << std::endl;
		}

		// Reader configuration object to store reader provider and reader unit selection.
		boost::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(new logicalaccess::ReaderConfiguration());

		// PC/SC
		readerConfig->setReaderProvider(logicalaccess::LibraryManager::getInstance()->getReaderProvider("PCSC"));

		if (readerConfig->getReaderProvider()->getRPType() == "PCSC" && readerConfig->getReaderProvider()->getReaderList().size() == 0)
		{
			std::cerr << "No readers on this system." << std::endl;
			return EXIT_FAILURE;
		}
		std::cout << readerConfig->getReaderProvider()->getReaderList().size() << " readers on this system." << std::endl;

		// Create the default reader unit. On PC/SC, we will listen on all readers.
		readerConfig->setReaderUnit(readerConfig->getReaderProvider()->createReaderUnit());

		boost::shared_ptr<logicalaccess::PCSCReaderUnit> readerunit = boost::dynamic_pointer_cast<logicalaccess::PCSCReaderUnit>(readerConfig->getReaderUnit());

		//boost::shared_ptr<logicalaccess::DESFireKey> keysecurity(new logicalaccess::DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
		//keysecurity->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_DES);
		//readerunit->setName("OMNIKEY CardMan 5x21-CL 0");
		//readerunit->setName("OMNIKEY 6321-CL 0");
		//readerunit->getPCSCConfiguration()->setSAMSecurityCheck(keysecurity, 4);
		//readerunit->getPCSCConfiguration()->setSAMType("SAM_AUTO");
		//readerunit->getPCSCConfiguration()->setSAMReaderName("OMNIKEY CardMan 5x21 0");
		//readerunit->getPCSCConfiguration()->setSAMReaderName("OMNIKEY 6321 0");

		unsigned char data[2048];
		memset(data, 0x00, sizeof(data));				

		std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;

		while (1)
		{
			readerConfig->getReaderUnit()->connectToReader();

			// Force card type here if you want to
			//readerConfig->getReaderUnit()->setCardType(CT_DESFIRE_EV1);

			std::cout << "Time start : " << time(NULL) << std::endl;
			if (readerConfig->getReaderUnit()->waitInsertion(15000))
			{
				
				if (readerConfig->getReaderUnit()->connect())
				{
				

					boost::shared_ptr<logicalaccess::Chip> chip = readerConfig->getReaderUnit()->getSingleChip();

					boost::shared_ptr<logicalaccess::DESFireISO7816Commands> desfirecommand = boost::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(chip->getCommands());
					

				/*	//changeKeyEntry
					boost::shared_ptr<logicalaccess::SAMKeyEntry> keyentry = boost::dynamic_pointer_cast<logicalaccess::SAMAV1ISO7816Commands>(desfirecommand->getSAMChip()->getCommands())->getKeyEntry(10);

					boost::shared_ptr<logicalaccess::DESFireKey> keySam(new logicalaccess::DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
					keySam->setKeyType(logicalaccess::DF_KEY_DES);
					boost::dynamic_pointer_cast<logicalaccess::SAMAV1ISO7816Commands>(desfirecommand->getSAMChip()->getCommands())->authentificateHost(keySam, 0);

					/*logicalaccess::KeyEntryUpdateSettings t;
					memset(&t, 1, sizeof(logicalaccess::KeyEntryUpdateSettings));*/
				/*	t.keyVa = 1;
					t.keyVb = 1;
					t.keyversionsentseparatly = 1;
					t.updateset = 1;	
					t.key_no_v_cek = 1;
					t.df_aid_keyno = 1;*/
					/*keyentry->setUpdateSettings(t);
					keyentry->getKeyEntryInformation().cekno = 0;
					keyentry->getKeyEntryInformation().cekv = 0;
					keyentry->getKeyEntryInformation().vera = 1;
					keyentry->getKeyEntryInformation().verb = 0;
					keyentry->getKeyEntryInformation().set[0] = 0x25;
					keyentry->getKeyEntryInformation().set[1] = 0x00;
					keyentry->getKeyEntryInformation().desfireAid[0] = 0xf5;
					keyentry->getKeyEntryInformation().desfireAid[1] = 0x13;
					keyentry->getKeyEntryInformation().desfireAid[2] = 0xb3;
					keyentry->getKeyEntryInformation().desfirekeyno = 1;
					std::vector<std::vector<unsigned char> > keys = keyentry->getKeysData();
					keys[0][0] = 0xaa; //2c06c87104344eeabf45181068ad697e
					keys[0] = logicalaccess::BufferHelper::fromHexString("2c06c87104344eeabf45181068ad697e");
					//keys[1][0] = 0xbb;
					keyentry->setKeysData(keys, logicalaccess::SAMKeyType::SAM_KEY_AES);

					logicalaccess::SET setest = keyentry->getSETStruct();

					keyentry->setSET(setest);
									//	keyentry->getKeyEntryInformation()->set[0] = 0x00;
					//ChangeKeyEntry
					boost::dynamic_pointer_cast<logicalaccess::SAMAV1ISO7816Commands>(desfirecommand->getSAMChip()->getCommands())->changeKeyEntry(10, keyentry, keySam);*/


					
					//Simple Auth
					boost::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02"));
					key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_DES);
					boost::shared_ptr<logicalaccess::KeyDiversification> keydiv(new logicalaccess::SagemKeyDiversification());
					key->setKeyDiversification(keydiv);
					boost::dynamic_pointer_cast<logicalaccess::DESFireProfile>(chip->getProfile())->setKey(4344143, 1, key); 

					desfirecommand->selectApplication(4344143);
					desfirecommand->authenticate(1, key);

					/*
					//Simple AuthHost
					boost::shared_ptr<logicalaccess::SAMAV1ISO7816Commands> samcmds = boost::dynamic_pointer_cast<logicalaccess::SAMAV1ISO7816Commands>(desfirecommand->getSAMChip()->getCommands());

					boost::shared_ptr<logicalaccess::DESFireKey> keysam(new logicalaccess::DESFireKey("aa 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
					keysam->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_AES);

					samcmds->authentificateHost(keysam, 1);*/

				/*	//Authen With SAM
					boost::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
					boost::shared_ptr<logicalaccess::SAMKeyStorage> samstorage(new logicalaccess::SAMKeyStorage());
					key->setKeyStorage(samstorage);
					key->setKeyVersion(1);
					samstorage->setKeySlot(1);
					key->setKeyType(logicalaccess::DF_KEY_AES);
					boost::dynamic_pointer_cast<logicalaccess::DESFireProfile>(chip->getProfile())->setKey(0xf513b3, 1, key); 

					desfirecommand->selectApplication(0xf513b3);
					desfirecommand->authenticate(1);*/

				/*	boost::shared_ptr<logicalaccess::DESFireKey> key2(new logicalaccess::DESFireKey());
					boost::shared_ptr<logicalaccess::SAMKeyStorage> samstoragee(new logicalaccess::SAMKeyStorage());
					samstoragee->setKeySlot(2);
					key2->setKeyStorage(samstoragee);
					key2->setKeyVersion(2);
					key2->setKeyType(logicalaccess::DF_KEY_AES);

					desfirecommand->changeKey(1, key2);*/

					readerConfig->getReaderUnit()->disconnect();
				}
				else
				{
					std::cout << "Error: cannot connect to the card." << std::endl;
				}

				std::cout << "Logical automatic card removal in 15 seconds..." << std::endl;

				if (!readerConfig->getReaderUnit()->waitRemoval(15000))
				{
					std::cerr << "Card removal forced." << std::endl;
				}

				std::cout << "Card removed." << std::endl;
			} else
			{
				std::cout << "No card inserted." << std::endl;
			}

			readerConfig->getReaderUnit()->disconnectFromReader();
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
