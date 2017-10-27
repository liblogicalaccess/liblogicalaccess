#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "pluginscards/iso15693/iso15693commands.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"
#include "logicalaccess/logs.hpp"
#include <vector>

void introduction()
{
    PRINT_TIME("This test target ISO15693 cards.");
    PRINT_TIME("It works properly on windows with OK5321 but is broken on Linux for this "
               "reader");
    PRINT_TIME("OK5427 partially works on both Linux and Windows");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME(
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("SystemInformation");
    LLA_SUBTEST_REGISTER("WriteBlock");
    LLA_SUBTEST_REGISTER("ReadBlock");
}

using namespace logicalaccess;

static void write_read_test(ReaderUnitPtr /*readerUnit*/,
                            std::shared_ptr<ISO15693Commands> cmd)
{
    uint8_t val = 0x00;
    for (int i = 1; i < 7; ++i)
    {
        cmd->writeBlock(i, {++val, ++val, ++val, ++val});
    }
    LLA_SUBTEST_PASSED("WriteBlock");

    val = 0x00;
    for (int i = 1; i < 7; ++i)
    {
        auto ret = cmd->readBlock(i);
        std::cout << "Read back (" << i << ") : " << ret << std::endl;
        auto tmp_vec = {++val, ++val, ++val, ++val};
        LLA_ASSERT(std::equal(ret.begin(), ret.end(), tmp_vec.begin()),
                   "We didn't read back what we wrote");
    }
    LLA_SUBTEST_PASSED("ReadBlock");
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("CHip identifier: "
               << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "ISO15693",
               "Chip is not a ISO15693, but is " + chip->getCardType() + " instead.");

    auto cmd = std::dynamic_pointer_cast<ISO15693Commands>(chip->getCommands());
    LLA_ASSERT(cmd, "Cannot retrieve correct command object");


    auto sysinfo = cmd->getSystemInformation();
    // Our unittest card is supposed to have the blocksize/nbblock available.
    // However, this doesn't work with OK5427 and doesn't work with OK5321 on Linux.
    if (pcsc_reader_unit_name(readerUnit) == "OKXX21" && get_os_name() == "Win")
    {
        LLA_ASSERT(sysinfo.hasVICCMemorySize, "Doesn't have VICCMemorySize information");
        LLA_ASSERT(sysinfo.blockSize == 4, "Unexpected block size");
        LLA_ASSERT(sysinfo.nbBlocks == 40, "Unexpected block number");
        LLA_SUBTEST_PASSED("SystemInformation");
    }

    if ((get_os_name() == "Win" && pcsc_reader_unit_name(readerUnit) == "OKXX21") ||
        pcsc_reader_unit_name(readerUnit) == "OKXX27")
    {
        write_read_test(readerUnit, cmd);
    }

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
