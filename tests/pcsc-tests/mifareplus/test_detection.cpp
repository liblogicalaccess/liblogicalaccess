//
// Created by xaqq on 6/25/15.
//
#include <pluginscards/mifare/mifareaccessinfo.hpp>
#include <pluginscards/mifareplus/mifarepluschip.hpp>
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target MifarePlus cards");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

using namespace logicalaccess;

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = pcsc_test_init();

    PRINT_TIME("Chip identifier: " <<
               logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));
    PRINT_TIME("Chip type = " << chip->getCardType());
    PRINT_TIME("Generic type = " << chip->getGenericCardType());

    LLA_ASSERT(chip->getCommands(), "No command object.");

   // auto mfpcmd = std::dynamic_pointer_cast<MifarePlusCommands>(chip->getCommands());
    //LLA_ASSERT(mfpcmd, "Invalid command object");

    auto mfp = std::dynamic_pointer_cast<MifarePlusChip>(chip);
    LLA_ASSERT(mfp, "Invalid chip object");

    PRINT_TIME("Security Level: " << mfp->getSecurityLevel());

    pcsc_test_shutdown(readerUnit);
    return EXIT_SUCCESS;
}
