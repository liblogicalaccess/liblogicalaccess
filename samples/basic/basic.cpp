/**
 * \file main.cpp
 * \author Maxime Chamley <maxime.chamley@islog.com>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include "logicalaccess/DynLibrary/LibraryManager.h"
#include "logicalaccess/ReaderProviders/ReaderConfiguration.h"
#include "logicalaccess/Cards/CardProvider.h"

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
		std::string readerSerialPort;

#ifdef LINUX
		readerSerialPort = "/dev/ttyUSB1";
#else
		readerSerialPort = "COM1";
#endif

		// Reader configuration object to store reader provider and reader unit selection.
		boost::shared_ptr<LOGICALACCESS::ReaderConfiguration> readerConfig(new LOGICALACCESS::ReaderConfiguration());

		// PC/SC
		readerConfig->setReaderProvider(LOGICALACCESS::LibraryManager::getInstance()->getReaderProvider("PCSC"));

		if (readerConfig->getReaderProvider()->getRPType() == "PCSC" && readerConfig->getReaderProvider()->getReaderList().size() == 0)
		{
			std::cerr << "No readers on this system." << std::endl;
			return EXIT_FAILURE;
		}
		else
		{
			std::cout << readerConfig->getReaderProvider()->getReaderList().size() << " readers on this system." << std::endl;			
		}

		// Create the default reader unit. On PC/SC, we will listen on all readers.
		readerConfig->setReaderUnit(readerConfig->getReaderProvider()->createReaderUnit());

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

					boost::shared_ptr<LOGICALACCESS::Chip> chip = readerConfig->getReaderUnit()->getSingleChip();
					std::cout << "Card type: " << chip->getCardType() << std::endl;

					std::vector<unsigned char> csn = readerConfig->getReaderUnit()->getNumber(chip);
					std::cout << "Card Serial Number : " << LOGICALACCESS::BufferHelper::getHex(csn) << std::endl;	

					std::cout << "Chip list:" << std::endl;
					std::vector<boost::shared_ptr<LOGICALACCESS::Chip>> chipList = readerConfig->getReaderUnit()->getChipList();
					for(std::vector<boost::shared_ptr<LOGICALACCESS::Chip>>::iterator i = chipList.begin(); i != chipList.end(); ++i)
					{
						std::cout << "\t" << LOGICALACCESS::BufferHelper::getHex(readerConfig->getReaderUnit()->getNumber((*i))) << std::endl;
					}

					boost::shared_ptr<LOGICALACCESS::CardProvider> cp = chip->getCardProvider();
					boost::shared_ptr<LOGICALACCESS::Profile> profile = chip->getProfile();

					// DO SOMETHING HERE
					// DO SOMETHING HERE
					// DO SOMETHING HERE

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
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
