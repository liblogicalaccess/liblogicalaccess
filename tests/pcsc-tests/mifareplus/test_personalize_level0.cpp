#include <pluginscards/mifare/mifareaccessinfo.hpp>
#include <pluginscards/mifareplus/MifarePlusSL0Commands.hpp>
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target MifarePlus cards in Security Level 0.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

using namespace logicalaccess;

bool is_trailing_block(int blkno)
{
    if (blkno % 4 == 3)
        return true;
    return false;
}

void test_write_perso_reject_invalid_access_bits(std::shared_ptr<MifarePlusSL0Commands> mpu)
{
    // WritePerso should refuse to write invalid access bits sector.
    std::array<uint8_t, 16> data = {0};
    LLA_ASSERT(mpu->writePerso(0, 3, data) == false,
               "WRITE PERSO did not reject invalid input.");
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = pcsc_test_init();

    PRINT_TIME("Chip identifier: " <<
               logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));
    PRINT_TIME("Chip type = " << chip->getCardType());

    LLA_ASSERT(chip->getCommands(), "No command object.");

    auto mpu = std::dynamic_pointer_cast<MifarePlusSL0Commands>(chip->getCommands());
    LLA_ASSERT(mpu, "Not Mifare Plus Utils");
    LLA_ASSERT(mpu->detectSecurityLevel() == 0, "Card is not in security level 0");

    test_write_perso_reject_invalid_access_bits(mpu);

    // rewrite data block with 0, assuming this a 4k card
    for (int i = 1; i < 0xFF; ++i)
    {
        if (is_trailing_block(i))
        {
            std::array<uint8_t, 16> data = {0};
            data[6] = 0xff;
            data[7] = 0x07;
            data[8] = 0x80;
            LLA_ASSERT(mpu->writePerso(0, i, data), "Write perso failed");
        }
        else
        {
            LLA_ASSERT(mpu->writePerso(0, i, {0}), "Write perso failed");
        }
    }

    // rewrite AES key A and B for all sectors
    for (int i = 0; i <= 0x3F; ++i)
        mpu->writePerso(0x40, i, {0});

    // Master card key
    mpu->writePerso(MF_CARD_MASTER_KEY, {0});
    mpu->writePerso(MF_CARD_CONFIGURATION_KEY, {0});
    try { mpu->writePerso(MF_L2_SWITCH_KEY, {0}); } // mifare plus S doesn't have this
    catch (...)
    {
        PRINT_TIME("Failed to set SL2 switch key. The card propably is MFP S");
    }
    mpu->writePerso(MF_L3_SWITCH_KEY, {0});
    mpu->writePerso(MF_L1_CARD_AUTH_KEY, {0});

    // OMG OMG
    // HOPE CARD WONT DIE
     //mpu->commitPerso();

    pcsc_test_shutdown(readerUnit);
    return EXIT_SUCCESS;
}
