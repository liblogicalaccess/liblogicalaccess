/**
 * \file main.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"

//#include "osdpreaderunitconfiguration.hpp"
//#include "osdpreaderunit.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "readercardadapters\elatecbufferparser.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "desfireev1nfctag4cardservice.hpp"
#include "pcscreaderunit.hpp"

#include "elatecreaderprovider.hpp"
#include "elatecreaderunitconfiguration.hpp"

#include "scielreaderprovider.hpp"
#include "scielreaderunitconfiguration.hpp"

#include "axesstmc13readerprovider.hpp"
#include "axesstmc13readerunitconfiguration.hpp"

#include "deisterreaderprovider.hpp"
#include "deisterreaderunitconfiguration.hpp"

#include "stidstrreaderunitconfiguration.hpp"
#include "stidstrreaderprovider.hpp"
#include "readercardadapters/desfireev1stidstrcommands.hpp"

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
		boost::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(new logicalaccess::ReaderConfiguration());
 
		// Set PCSC ReaderProvider by calling the Library Manager which will load the function from the corresponding plug-in
	/*	 readerConfig->setReaderProvider(logicalaccess::LibraryManager::getInstance()->getReaderProvider("PCSC"));
		readerConfig->setReaderUnit(readerConfig->getReaderProvider()->createReaderUnit());
		boost::shared_ptr<logicalaccess::PCSCReaderUnit> readerUnit = boost::dynamic_pointer_cast<logicalaccess::PCSCReaderUnit>(readerConfig->getReaderUnit());
		readerUnit->setName("OMNIKEY CardMan 5x21-CL 0");*/
 
		/*boost::shared_ptr<logicalaccess::ElatecReaderUnitConfiguration> readerUnitConfig(new logicalaccess::ElatecReaderUnitConfiguration());
		boost::shared_ptr<logicalaccess::ElatecReaderUnit> readerUnit(new logicalaccess::ElatecReaderUnit());
		readerUnit->setConfiguration(readerUnitConfig);
		//readerUnit->setCardType("EM4102");
		boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())
			->setSerialPort(boost::shared_ptr<logicalaccess::SerialPortXml>(new logicalaccess::SerialPortXml("COM4")));*/


		/*boost::shared_ptr<logicalaccess::DeisterReaderUnitConfiguration> readerUnitConfig(new logicalaccess::DeisterReaderUnitConfiguration());
		boost::shared_ptr<logicalaccess::DeisterReaderUnit> readerUnit(new logicalaccess::DeisterReaderUnit());
		readerUnit->setConfiguration(readerUnitConfig);
		//readerUnit->setCardType("EM4102");
		boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())
			->setSerialPort(boost::shared_ptr<logicalaccess::SerialPortXml>(new logicalaccess::SerialPortXml("COM62")));*/
		//boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())->setPortBaudRate(9600);


	/*	boost::shared_ptr<logicalaccess::SCIELReaderUnitConfiguration> readerUnitConfig(new logicalaccess::SCIELReaderUnitConfiguration());
		boost::shared_ptr<logicalaccess::SCIELReaderUnit> readerUnit(new logicalaccess::SCIELReaderUnit());
		readerUnit->setConfiguration(readerUnitConfig);
		readerUnit->setCardType("EM4102");
		boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())
			->setSerialPort(boost::shared_ptr<logicalaccess::SerialPortXml>(new logicalaccess::SerialPortXml("COM3")));
		boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())->setPortBaudRate(9600);*/

		boost::shared_ptr<logicalaccess::STidSTRReaderUnitConfiguration> readerUnitConfig(new logicalaccess::STidSTRReaderUnitConfiguration());
		boost::shared_ptr<logicalaccess::STidSTRReaderUnit> readerUnit(new logicalaccess::STidSTRReaderUnit());
		readerUnit->setConfiguration(readerUnitConfig);
		boost::dynamic_pointer_cast<logicalaccess::SerialPortDataTransport>(readerUnit->getDataTransport())
			->setSerialPort(boost::shared_ptr<logicalaccess::SerialPortXml>(new logicalaccess::SerialPortXml("COM3")));

		std::size_t start = std::clock();
        while (1)
        {
			//readerUnit->connectToReader();
			//readerUnit->disconnectFromReader();
            if (readerUnit->connectToReader())
			{
				std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tConnected to reader" << std::endl;
				while (true)
				{
					std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tStartWaitInsertation" << std::endl;
					if (readerUnit->waitInsertion(2000))
					{
						std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tEndWaitInsertation" << std::endl;
						if (readerUnit->connect())
						{
							std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tconnect" << std::endl;
							boost::shared_ptr<logicalaccess::Chip> chip = readerUnit->getSingleChip();
							std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tGetSingleChip" << std::endl;
							if (chip)
							{
								while (1)
								{
								chip->getRootLocationNode();
								std::cout << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()) << std::endl;
								//boost::shared_ptr<logicalaccess::DESFireEV1STidSTRCommands> defirecommands(boost::dynamic_pointer_cast<logicalaccess::DESFireEV1STidSTRCommands>(chip->getCommands()));
								std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tDone" << std::endl;
								}
						/*		defirecommands->getFileIDs();
								defirecommands->selectApplication(1313);*/
								//defirecommands->getFileIDs();
								
								/*		boost::shared_ptr<logicalaccess::ISO7816ISO7816Commands> iso7816command(boost::dynamic_pointer_cast<logicalaccess::ISO7816ISO7816Commands>(chip->getCommands()));
								unsigned char data[128];
								size_t datalength = 59;
								iso7816command->selectFile(0xe105);
								iso7816command->selectFile(0xe104);
								iso7816command->readBinary(data, datalength, 60, 0xe104);

								boost::shared_ptr<logicalaccess::DESFireEV1NFCTag4CardService> cardService = boost::dynamic_pointer_cast<logicalaccess::DESFireEV1NFCTag4CardService>(chip->getService(logicalaccess::CardServiceType::CST_NFC_TAG));
								boost::shared_ptr<logicalaccess::NdefMessage> myMessage = cardService->readNDEFFile();
								*/

							}
							readerUnit->disconnect();
						}
						std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tStartWaitRemoval" << std::endl;
						readerUnit->waitRemoval(2000);
						std::cout << (double)((std::clock() - start) / (double) CLOCKS_PER_SEC) << "\tEndWaitRemoval" << std::endl;

					}
				}
				readerUnit->disconnectFromReader();
			}           
        }
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
