#include <logicalaccess/bufferhelper.hpp>
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"
#include <cassert>
#include <pluginscards/mifareplus/MifarePlusSL0Commands.hpp>

void introduction()
{
    PRINT_TIME("This test target MifarePlus cards.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

int main(int ac, char **av)
{
    using namespace logicalaccess;

    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = pcsc_test_init();

    PRINT_TIME("Chip identifier: "
               << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));
    PRINT_TIME("Chip type = " << chip->getCardType());

    // LLA_ASSERT(chip->getCardType() == "GENERIC_T_CL",
    //               "Chip is not GENERIC_T_CL");

    LLA_ASSERT(chip->getCommands(), "No command object.");

    using namespace logicalaccess;
    auto mpu = std::dynamic_pointer_cast<MifarePlusSL0Commands>(chip->getCommands());
    assert(mpu);
    PRINT_TIME("Boap: " << mpu->detectSecurityLevel());

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
