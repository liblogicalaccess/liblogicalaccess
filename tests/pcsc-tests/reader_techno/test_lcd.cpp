#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/readerproviders/lcddisplay.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"


using namespace logicalaccess;

void introduction()
{
    PRINT_TIME("This test checks that the LCD screen for ACR1222L works properly.");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("DisplaySomeNumbers");
    LLA_SUBTEST_REGISTER("DisplayPerRow");
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;

    // Reader configuration object to store reader provider and reader unit selection.
    std::shared_ptr<ReaderConfiguration> readerConfig(
            new ReaderConfiguration());

    // Set PCSC ReaderProvider by calling the Library Manager which will load the function from the corresponding plug-in
    provider = LibraryManager::getInstance()->getReaderProvider("PCSC");
    LLA_ASSERT(provider, "Cannot get PCSC provider");
    readerConfig->setReaderProvider(provider);

    for (auto &r : readerConfig->getReaderProvider()->getReaderList())
    {
        PRINT_TIME("Available reader: " << r->getConnectedName());
    }

    readerUnit = readerConfig->getReaderProvider()->getReaderList().front();
    LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");

    PRINT_TIME("Reader name: " << readerUnit->getName());
    PRINT_TIME("Connected Reader Name: " << readerUnit->getConnectedName());

    auto lcd = readerUnit->getLCDDisplay();
    LLA_ASSERT(lcd, "No LCD display.");

	std::string msg;
    for (int x = 0; x < 3; x++)
        for (int i = 0 ; i < 10; i++)
            msg.push_back(i + '0');

    readerUnit->getLCDDisplay()->setMessage(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    LLA_SUBTEST_PASSED("DisplaySomeNumbers");

    readerUnit->getLCDDisplay()->clear();
    readerUnit->getLCDDisplay()->setMessage(0, "LAMASTICOT");
    readerUnit->getLCDDisplay()->setMessage(1, "LAMASTICOT");
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    LLA_SUBTEST_PASSED("DisplayPerRow");
    readerUnit->getLCDDisplay()->clear();

    return EXIT_SUCCESS;
}
