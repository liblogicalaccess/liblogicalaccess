#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcommands.hpp>

#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>

void introduction()
{
    PRINT_TIME("This test target MifareUltraLight cards.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("WritePage");
    LLA_SUBTEST_REGISTER("ReadPage");
    LLA_SUBTEST_REGISTER("WriteRead");
}

int main(int ac, char **av)
{
    using namespace logicalaccess;

    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("Chip identifier: "
               << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "MifareUltralight",
               "Chip is not an MifareUltralight(C), but is " + chip->getCardType() +
                   " instead.");
    auto cmd = std::dynamic_pointer_cast<MifareUltralightCommands>(chip->getCommands());
    LLA_ASSERT(cmd, "Cannot get command from chip");

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
