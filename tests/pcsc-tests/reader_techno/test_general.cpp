#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"


using namespace logicalaccess;

void introduction()
{
    PRINT_TIME("This test chekcs that we properly disable "
               "or enable type of cards");

    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("ChangeCardTechno");
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;

    // Reader configuration object to store reader provider and reader unit
    // selection.
    std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
        new logicalaccess::ReaderConfiguration());

    // Set PCSC ReaderProvider by calling the Library Manager which will load
    // the function from the corresponding plug-in
    provider =
        logicalaccess::LibraryManager::getInstance()->getReaderProvider("PCSC");
    LLA_ASSERT(provider, "Cannot get PCSC provider");
    readerConfig->setReaderProvider(provider);
    readerUnit = readerConfig->getReaderProvider()->createReaderUnit();

    LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");
    PRINT_TIME("Connected to reader");


    PRINT_TIME("StartWaitInsertation");
    LLA_ASSERT(readerUnit->waitInsertion(15000), "waitInsertion failed");
    PRINT_TIME("EndWaitInsertation");
    LLA_ASSERT(readerUnit->connect(), "Failed to connect");
    PRINT_TIME("Connected");
	
    TechnoBitset current;
    current = readerUnit->getCardTechnologies();

    LLA_ASSERT(current & Techno::ISO_14443_A, "ISO 14443_A is disabled");
    LLA_ASSERT(current & Techno::ISO_14443_B, "ISO 14443_B is disabled");

    current = 0;
    readerUnit->setCardTechnologies(current);

    current = readerUnit->getCardTechnologies();

    LLA_ASSERT(current == 0, "ISO_14443_A or ISO_14443_B technology is enabled.");

	current |= Techno::ISO_14443_A | Techno::ISO_14443_B;
    std::cout << "current  =" << current << std::endl;
    readerUnit->setCardTechnologies(current);

    current = readerUnit->getCardTechnologies();
    std::cout << "current  =" << current << std::endl;

	LLA_ASSERT(current & (Techno::ISO_14443_A | Techno::ISO_14443_B),
               "ISO_14443_A or ISO_14443_B technology is disabled.");

	LLA_SUBTEST_PASSED("ChangeCardTechno");

    return EXIT_SUCCESS;
}
