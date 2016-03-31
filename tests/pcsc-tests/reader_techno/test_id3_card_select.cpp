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

#include "pluginsreaderproviders/iso7816/commands/desfireev1iso7816commands.hpp"
#include "pluginscards/desfire/desfireev1commands.hpp"
#include "pluginscards/desfire/desfireev1chip.hpp"
#include "pluginsreaderproviders/pcsc/readers/id3readerunit.hpp"
#include "pluginsreaderproviders/pcsc/readercardadapters/pcscreadercardadapter.hpp"

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

	std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
		new logicalaccess::ReaderConfiguration());
	provider = LibraryManager::getInstance()->getReaderProvider("PCSC");
	LLA_ASSERT(provider, "Cannot get PCSC provider");
	readerConfig->setReaderProvider(provider);

	for (auto &r : readerConfig->getReaderProvider()->getReaderList())
	{
		PRINT_TIME("Available reader: " << r->getConnectedName());
	}

	readerUnit = std::dynamic_pointer_cast<PCSCReaderUnit>(readerConfig->getReaderProvider()->getReaderList()[0]);
	LLA_ASSERT(readerUnit, "No PCSC reader unit.");
	LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");

	PRINT_TIME("Reader name: " << readerUnit->getName());
	PRINT_TIME("Connected Reader Name: " << readerUnit->getConnectedName());

	auto id3 = std::dynamic_pointer_cast<ID3ReaderUnit>(readerUnit);
	LLA_ASSERT(id3, "Reader unit is not ID3.");

    id3->connectToReader();
	LLA_ASSERT(id3->waitInsertion(10000), "No card inserted");
    id3->connect();
    PRINT_TIME("CSN : " << id3->getCardSerialNumber());

    for (auto c : id3->getChipList())
    {
        std::cout << "Chip type: " << c->getCardType() << ", identifier: "
            << c->getChipIdentifier() << std::endl;
        id3->selectCard(c);
        std::cout << "FROM READER: " << id3->getCardSerialNumber() << std::endl;
    }
    id3->disconnect();

	return 0;
}