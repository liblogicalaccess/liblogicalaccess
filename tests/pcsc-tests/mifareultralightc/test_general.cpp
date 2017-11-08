#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcommands.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightccommands.hpp>

#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>

void introduction()
{
    PRINT_TIME("This test target MifareUltraLightC cards.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("Authenticate");
    LLA_SUBTEST_REGISTER("WritePage");
    LLA_SUBTEST_REGISTER("ReadPage");
    LLA_SUBTEST_REGISTER("WriteRead");
}

std::tuple<logicalaccess::ReaderProviderPtr, logicalaccess::ReaderUnitPtr,
           logicalaccess::ChipPtr>
init()
{
    // Reader configuration object to store reader provider and reader unit selection.
    std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
        new logicalaccess::ReaderConfiguration());

    // Set PCSC ReaderProvider by calling the Library Manager which will load the function
    // from the corresponding plug-in
    auto provider =
        logicalaccess::LibraryManager::getInstance()->getReaderProvider("PCSC");
    LLA_ASSERT(provider, "Cannot get PCSC provider");
    readerConfig->setReaderProvider(provider);

    auto readerUnit = readerConfig->getReaderProvider()->createReaderUnit();
    readerUnit->setCardType(
        "MifareUltralightC"); // 5321 doesnt properly detect between MifareUltraLight
    // and MifareUltraLightC.

    LLA_ASSERT(readerUnit, "Cannot create reader unit");

    LLA_ASSERT(readerUnit->connectToReader(), "Cannot connect to reader");
    PRINT_TIME("Connected to reader");
    PRINT_TIME("StartWaitInsertation");

    LLA_ASSERT(readerUnit->waitInsertion(15000), "waitInsertion failed");
    PRINT_TIME("EndWaitInsertation");

    LLA_ASSERT(readerUnit->connect(), "Failed to connect");
    PRINT_TIME("Connected");

    auto chip = readerUnit->getSingleChip();
    LLA_ASSERT(chip, "getSingleChip() returned NULL");
    PRINT_TIME("GetSingleChip");

    return make_tuple(provider, readerUnit, chip);
}

int main(int ac, char **av)
{
    using namespace logicalaccess;
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = init();

    PRINT_TIME("CHip identifier: "
               << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "MifareUltralightC",
               "Chip is not an MifareUltralight(C), but is " + chip->getCardType() +
                   " instead.");
    auto cmd = std::dynamic_pointer_cast<MifareUltralightCommands>(chip->getCommands());
    LLA_ASSERT(cmd, "Cannot get command from chip");

    auto cmdUltraC =
        std::dynamic_pointer_cast<MifareUltralightCCommands>(chip->getCommands());
    LLA_ASSERT(cmdUltraC, "Cannot cast command to MifareUltralightCCommands");

    // std::shared_ptr<logicalaccess::TripleDESKey> key(new
    // logicalaccess::TripleDESKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
    std::shared_ptr<TripleDESKey> key;
    cmdUltraC->authenticate(key);
    LLA_SUBTEST_PASSED("Authenticate");

    ByteVector data(4), tmp;
    data[0] = 0x11;
    data[3] = 0xff;
    cmd->writePage(10, data);
    LLA_SUBTEST_PASSED("WritePage");

    tmp = cmd->readPage(10);
    LLA_SUBTEST_PASSED("ReadPage");


    LLA_ASSERT(std::equal(data.begin(), data.end(), tmp.begin()),
               "read and write data are different!");
    LLA_SUBTEST_PASSED("WriteRead");


    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
