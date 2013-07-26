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
#include "commands/samav2iso7816commands.hpp"
#include "commands/desfireiso7816commands.hpp"
#include "desfirechip.hpp"
#include "samchip.hpp"
#include "samav2keyentry.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

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
		
		readerunit->setName("OMNIKEY 6321-CL 0");
		readerunit->getPCSCConfiguration()->setSAMType("SAM_AV2");
		readerunit->getPCSCConfiguration()->setSAMReaderName("OMNIKEY 6321 0");


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
					std::cout << "Card inserted on reader \"" << readerConfig->getReaderUnit()->getConnectedName() << "\"." << std::endl;

					boost::shared_ptr<logicalaccess::Chip> chip = readerConfig->getReaderUnit()->getSingleChip();
					std::cout << "Card type: " << chip->getCardType() << std::endl;
					//set storage key SAM
					boost::shared_ptr<logicalaccess::DESFireLocation> location(new logicalaccess::DESFireLocation());
					boost::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
					boost::shared_ptr<logicalaccess::KeyStorage> samstorage(new logicalaccess::SAMKeyStorage());
					key->setKeyStorage(samstorage);
					boost::dynamic_pointer_cast<logicalaccess::DESFireProfile>(chip->getProfile())->setKey(1313, 1, key); 

					std::vector<unsigned char> csn = readerConfig->getReaderUnit()->getNumber(chip);
					std::cout << "Card Serial Number : " << logicalaccess::BufferHelper::getHex(csn) << std::endl;	

					std::cout << "Chip list:" << std::endl;
					std::vector<boost::shared_ptr<logicalaccess::Chip>> chipList = readerConfig->getReaderUnit()->getChipList();
					for(std::vector<boost::shared_ptr<logicalaccess::Chip>>::iterator i = chipList.begin(); i != chipList.end(); ++i)
					{
						std::cout << "\t" << logicalaccess::BufferHelper::getHex(readerConfig->getReaderUnit()->getNumber((*i))) << std::endl;
					}

					//boost::shared_ptr<logicalaccess::Profile> profile = chip->getProfile();

					boost::shared_ptr<logicalaccess::PCSCReaderUnit> storage = boost::dynamic_pointer_cast<logicalaccess::PCSCReaderUnit>(readerConfig->getReaderUnit());
					
					boost::shared_ptr<logicalaccess::DESFireISO7816Commands> desfirecommand = boost::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(readerConfig->getReaderUnit()->getSingleChip()->getCommands());
					
					
					boost::dynamic_pointer_cast<logicalaccess::SAMAV2ISO7816Commands>(desfirecommand->getSAMChip()->getCommands())->GetVersion();


					desfirecommand->selectApplication(1313);
					desfirecommand->authenticate(1);
					// DO SOMETHING HERE
					// DO SOMETHING HERE
					// DO SOMETHING HERE
					boost::shared_ptr<logicalaccess::SAMAV2KeyEntry> keyentry = boost::dynamic_pointer_cast<logicalaccess::SAMAV2ISO7816Commands>(desfirecommand->getSAMChip()->getCommands())->GetKeyEntry(1);



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
