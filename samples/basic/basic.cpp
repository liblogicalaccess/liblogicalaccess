/**
 * \file main.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/nfctag/nfctagcardservice.hpp"
#include "logicalaccess/cards/locationnode.hpp"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

std::string UCharCollectionToHexString(std::vector<unsigned char> uchars)
{
	std::stringstream ss;
	for (unsigned int i = 0; i < uchars.size(); i++)
	{
		ss << std::hex << (int)uchars[i];
	}
	return ss.str();
}

/**
 * \brief The application entry point.
 * \param argc The arguments count.
 * \param argv The arguments.
 */
int main(int , char**)
{
    try
    {
		logicalaccess::LibraryManager* manager = logicalaccess::LibraryManager::getInstance();

		std::shared_ptr<logicalaccess::ReaderProvider> readerProvider = manager->getReaderProvider("PCSC");

		std::shared_ptr<logicalaccess::ReaderUnit> readerUnit = readerProvider->createReaderUnit();

		std::vector<std::shared_ptr<logicalaccess::ReaderUnit>> readerList = readerProvider->getReaderList();

		if (readerProvider->getRPType() == "PCSC" && readerList.size() == 0)
		{
			std::cout << "No readers on this system." << std::endl;
			exit(1);
		}

        std::cout << readerList.size() << " readers on this system." << std::endl;

		if (readerUnit->connectToReader())
		{
			std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;
			if (readerUnit->waitInsertion(15000))
			{
				if (readerUnit->connect())
				{
					std::cout << "Card inserted on reader " << readerUnit->getConnectedName() << std::endl;
					std::shared_ptr<logicalaccess::Chip> chip = readerUnit->getSingleChip();
					std::cout << "\tCSN: " << UCharCollectionToHexString(chip->getChipIdentifier()) << std::endl;
					std::cout << "\tChip Name: " << chip->getCardType() << std::endl;

					readerUnit->disconnect();
				}

				std::cout << "Logical automatic card removal in 15 seconds..." << std::endl;
				if (!readerUnit->waitRemoval(15000))
				{
					std::cout << "Card removal forced." << std::endl;
				}
				std::cout << "Card removed." << std::endl;
			}
			else
			{
				std::cout << "No card inserted." << std::endl;
			}
			readerUnit->disconnectFromReader();
		}
		else
		{
			std::cout << "Unable to connect to the reader" << std::endl;
		}





    //    // List available reader units
    //    int ruindex = 0;
    //    const logicalaccess::ReaderList readers = readerConfig->getReaderProvider()->getReaderList();
    //    std::cout << "Please select index of the reader unit to use:" << std::endl;
    //    for (int i = 0; i < readers.size(); ++i)
    //    {
    //        std::cout << "\t" << i << ". " << readers.at(i)->getName() << std::endl;
    //    }
    //    std::cin >> ruindex;
	//
    //    if (ruindex < 0 || ruindex >= readers.size())
    //    {
    //        std::cerr << "Selected reader unit outside range." << std::endl;
    //        return EXIT_FAILURE;
    //    }
	//
    //    readerConfig->setReaderUnit(readers.at(ruindex));
    //    std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;
	//
    //    while (1)
    //    {
    //        readerConfig->getReaderUnit()->connectToReader();
	//
    //        // Force card type here if you want to
    //        //readerConfig->getReaderUnit()->setCardType(CT_DESFIRE_EV1);
	//
    //        std::cout << "Time start : " << time(NULL) << std::endl;
    //        if (readerConfig->getReaderUnit()->waitInsertion(15000))
    //        {
    //            
    //            if (readerConfig->getReaderUnit()->connect())
    //            {
    //                std::cout << "Card inserted on reader \"" << readerConfig->getReaderUnit()->getConnectedName() << "\"." << std::endl;
	//
    //                std::shared_ptr<logicalaccess::Chip> chip = readerConfig->getReaderUnit()->getSingleChip();
    //                std::cout << "Card type: " << chip->getCardType() << std::endl;
	//
    //                std::vector<unsigned char> csn = readerConfig->getReaderUnit()->getNumber(chip);
    //                std::cout << "Card Serial Number : " << logicalaccess::BufferHelper::getHex(csn) << std::endl;
	//
    //                std::shared_ptr<logicalaccess::LocationNode> node = chip->getRootLocationNode();
    //                if (node)
    //                {
    //                    std::cout << "Root Location Node : " << node->getName() << std::endl;
    //                }
	//
    //                std::cout << "Complete chip list:" << std::endl;
    //                std::vector<std::shared_ptr<logicalaccess::Chip>> chipList = readerConfig->getReaderUnit()->getChipList();
    //                for(std::vector<std::shared_ptr<logicalaccess::Chip>>::iterator i = chipList.begin(); i != chipList.end(); ++i)
    //                {
    //                    std::cout << "\t" << logicalaccess::BufferHelper::getHex(readerConfig->getReaderUnit()->getNumber((*i))) << std::endl;
    //                }
	//
    //                // DO SOMETHING HERE
	//				// DO SOMETHING HERE
	//				// DO SOMETHING HERE
	//
    //                readerConfig->getReaderUnit()->disconnect();
    //            }
    //            else
    //            {
    //                std::cout << "Error: cannot connect to the card." << std::endl;
    //            }
	//
    //            std::cout << "Logical automatic card removal in 15 seconds..." << std::endl;
	//
    //            if (!readerConfig->getReaderUnit()->waitRemoval(15000))
    //            {
    //                std::cerr << "Card removal forced." << std::endl;
    //            }
	//
    //            std::cout << "Card removed." << std::endl;
    //        } else
    //        {
    //            std::cout << "No card inserted." << std::endl;
    //        }
    //    }
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}