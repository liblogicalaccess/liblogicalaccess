#include <pluginsreaderproviders/pcsc/readers/acsacr1222lledbuzzerdisplay.hpp>
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
    PRINT_TIME("Test the leds of the ACR1222L reader.");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("TurnLedOn");
    LLA_SUBTEST_REGISTER("TurnLedOff");
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

    auto acs_led_buzzer = std::dynamic_pointer_cast<ACSACR1222LLEDBuzzerDisplay>(readerUnit->getLEDBuzzerDisplay());
    LLA_ASSERT(acs_led_buzzer, "LEDBuzzer control is not ACS specific.");

    PRINT_TIME("All led will be turned on for 4 seconds");
    acs_led_buzzer->setOrangeLed(true);
    acs_led_buzzer->setGreenLed(true);
    acs_led_buzzer->setRedLed(true);
    acs_led_buzzer->setBlueLed(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	LLA_SUBTEST_PASSED("TurnLedOn");

    PRINT_TIME("Turning led off");
    acs_led_buzzer->setOrangeLed(false);
    acs_led_buzzer->setGreenLed(false);
    acs_led_buzzer->setRedLed(false);
    acs_led_buzzer->setBlueLed(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	LLA_SUBTEST_PASSED("TurnLedOff");

    return EXIT_SUCCESS;
}
