/**
 * \file main.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/services/nfctag/nfctagcardservice.hpp>
#include <logicalaccess/services/challenge/challengecardservice.hpp>
#include <logicalaccess/cards/locationnode.hpp>

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include "logicalaccess/cards/pkcskeystorage.hpp"
#include "logicalaccess/plugins/cards/desfire/desfirekey.hpp"
#include "logicalaccess/plugins/cards/desfire/desfireev1commands.hpp"
#include "logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp"
#include "logicalaccess/plugins/cards/yubikey/yubikeycommands.hpp"

/**
 * \brief The application entry point.
 * \param argc The arguments count.
 * \param argv The arguments.
 */
int main(int, char **)
{
    try
    {
        std::vector<std::string> readerList =
            logicalaccess::LibraryManager::getInstance()->getAvailableReaders();
        std::cout << "Available reader plug-ins (" << readerList.size()
                  << "):" << std::endl;
        for (std::vector<std::string>::iterator it = readerList.begin();
             it != readerList.end(); ++it)
        {
            std::cout << "\t" << (*it) << std::endl;
        }

        std::vector<std::string> cardList =
            logicalaccess::LibraryManager::getInstance()->getAvailableCards();
        std::cout << "Available card plug-ins (" << cardList.size() << "):" << std::endl;
        for (std::vector<std::string>::iterator it = cardList.begin();
             it != cardList.end(); ++it)
        {
            std::cout << "\t" << (*it) << std::endl;
        }

        // Reader configuration object to store reader provider and reader unit selection.
        std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
            new logicalaccess::ReaderConfiguration());

        // PC/SC
        std::string rpstr = "PCSC";
        std::cout << "Please type the reader plug-in to use:" << std::endl;
        std::cin >> rpstr;

        readerConfig->setReaderProvider(
            logicalaccess::LibraryManager::getInstance()->getReaderProvider(rpstr));

        if (readerConfig->getReaderProvider() == nullptr)
        {
            std::cerr << "No reader provider" << std::endl;
            return EXIT_FAILURE;
        }

        if (readerConfig->getReaderProvider()->getRPType() == "PCSC" &&
            readerConfig->getReaderProvider()->getReaderList().size() == 0)
        {
            std::cerr << "No readers on this system." << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << readerConfig->getReaderProvider()->getReaderList().size()
                  << " readers on this system." << std::endl;

        // List available reader units
        int ruindex = 0;
        const logicalaccess::ReaderList readers =
            readerConfig->getReaderProvider()->getReaderList();
        std::cout << "Please select index of the reader unit to use:" << std::endl;
        for (int i = 0; i < static_cast<int>(readers.size()); ++i)
        {
            std::cout << "\t" << i << ". " << readers.at(i)->getName() << std::endl;
        }
        std::cin >> ruindex;

        if (ruindex < 0 || ruindex >= static_cast<int>(readers.size()))
        {
            std::cerr << "Selected reader unit outside range." << std::endl;
            return EXIT_FAILURE;
        }

        readerConfig->setReaderUnit(readers.at(ruindex));
        std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;

        while (1)
        {
            readerConfig->getReaderUnit()->connectToReader();

            // Force card type here if you want to
            readerConfig->getReaderUnit()->setCardType("DESFireEV1");

            std::cout << "Time start : " << time(NULL) << std::endl;
            if (readerConfig->getReaderUnit()->waitInsertion(15000))
            {

                if (readerConfig->getReaderUnit()->connect())
                {
                    std::cout << "Card inserted on reader \""
                              << readerConfig->getReaderUnit()->getConnectedName()
                              << "\"." << std::endl;

                    std::shared_ptr<logicalaccess::Chip> chip =
                        readerConfig->getReaderUnit()->getSingleChip();
                    std::cout << "Card type: " << chip->getCardType() << std::endl;

                    std::vector<unsigned char> csn =
                        readerConfig->getReaderUnit()->getNumber(chip);
                    std::cout << "Card Serial Number : "
                              << logicalaccess::BufferHelper::getHex(csn) << std::endl;

                    /*auto ykcmd = std::dynamic_pointer_cast<logicalaccess::YubikeyCommands>(chip->getCommands());
                    auto selectresp = ykcmd->selectYubikeyOATH();
                    std::cout << "Name: " << logicalaccess::BufferHelper::getHex(selectresp.name) << std::endl;
                    std::cout << "Challenge: " << logicalaccess::BufferHelper::getHex(selectresp.challenge) << std::endl;*/

                    readerConfig->getReaderUnit()->disconnect();
                }
                else
                {
                    std::cerr << "Cannot connect to the card." << std::endl;
                }

                std::cout << "Logical automatic card removal in 15 seconds..."
                          << std::endl;

                if (!readerConfig->getReaderUnit()->waitRemoval(15000))
                {
                    std::cerr << "Card removal forced." << std::endl;
                }

                std::cout << "Card removed." << std::endl;
            }
            else
            {
                std::cout << "No card inserted." << std::endl;
            }
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << "ERROR: " << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
