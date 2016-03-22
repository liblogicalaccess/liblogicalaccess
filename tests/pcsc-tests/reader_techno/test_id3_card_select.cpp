#include <pluginscards/prox/proxlocation.hpp>
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

//#include "pluginsreaderproviders/iso7816/commands/desfireev1iso7816commands.hpp"
#include "pluginscards/desfire/desfireev1commands.hpp"
#include "pluginscards/desfire/desfireev1chip.hpp"
#include "pluginsreaderproviders/pcsc/readers/id3readerunit.hpp"
//#include "pluginsreaderproviders/pcsc/readercardadapters/pcscreadercardadapter.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("Test the special capability of the ID3 reader.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

int main(int ac, char **av)
{
    using namespace logicalaccess;
	prologue(ac, av);
	introduction();
	ReaderProviderPtr provider;
	std::shared_ptr<PCSCReaderUnit> readerUnit;
	ChipPtr chip;

	std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
		new logicalaccess::ReaderConfiguration());
	provider = LibraryManager::getInstance()->getReaderProvider("PCSC");
	LLA_ASSERT(provider, "Cannot get PCSC provider");
	readerConfig->setReaderProvider(provider);

	for (auto &r : readerConfig->getReaderProvider()->getReaderList())
	{
		PRINT_TIME("Available reader: " << r->getConnectedName());
	}

	readerUnit = std::dynamic_pointer_cast<PCSCReaderUnit>(readerConfig->getReaderProvider()->getReaderList().front());
	LLA_ASSERT(readerUnit, "No PCSC reader unit.");
	LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");

	PRINT_TIME("Reader name: " << readerUnit->getName());
	PRINT_TIME("Connected Reader Name: " << readerUnit->getConnectedName());

	auto id3 = std::dynamic_pointer_cast<ID3ReaderUnit>(readerUnit);
	LLA_ASSERT(id3, "Reader unit is not ID3.");

	LLA_ASSERT(id3->waitInsertion(10000), "No card inserted");
	id3->connect();
	
	for (auto &chip_info : id3->listCards())
	{
		PRINT_TIME("Card Info: " << chip_info.identifier_);
	}

	id3->selectCard(1);
	id3->selectCard(0);

		return 0;
/*
	auto sc = [&](const std::vector<uint8_t> &d) { return id3->getDefaultPCSCReaderCardAdapter()->sendCommand(d); };
	auto ret = sc({ 0xFF, 0x9F, 0x00, 0x00, 0x01, 0x09});
	std::cout << "Ret: " << ret << std::endl;

	ret = sc({ 0xFF, 0x9F, 0x00, 0x00, 0x02, 0x0A, 0x00 });
	std::cout << "Ret: " << ret << std::endl;

	ret = sc({ 0xFF, 0xCA, 0x00, 0x00, 0x00 });
	std::cout << "Ret: " << ret << std::endl;

	ret = sc({ 0xFF, 0x9F, 0x00, 0x00, 0x02, 0x0A, 0x01 });
	std::cout << "Ret: " << ret << std::endl;

	ret = sc({ 0xFF, 0xCA, 0x00, 0x00, 0x00 });
	std::cout << "Ret: " << ret << std::endl;
	return 0;*/
}