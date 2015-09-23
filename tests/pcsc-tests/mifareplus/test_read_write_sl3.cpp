#include <pluginscards/mifareplus/mifareplussl3commands.hpp>
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "pluginscards/mifare/mifarechip.hpp"
#include "pluginscards/mifareplus/mifareplusaccessinfo_sl1.hpp"
#include "logicalaccess/cards/aes128key.hpp"

#include "logicalaccess/logs.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target MifarePlus cards in Security Level 3.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("WriteService");
    LLA_SUBTEST_REGISTER("ReadService");
    LLA_SUBTEST_REGISTER("CorrectWriteRead");
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("Chip identifier: " <<
               logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "MifarePlus_SL3_2K",
               "Chip is not a MifarePlus_SL3_2K, but is " + chip->getCardType() +
               " instead.");

    // Sector Key:
    std::shared_ptr<logicalaccess::AES128Key> aes_key = std::make_shared<logicalaccess::AES128Key>();
    //aes_key->fromString("ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff");
    //aes_key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    aes_key->fromString("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
 //   aes_key->fromString("000102030405060708090A0B0C0D0E0F");
    //                     000102030405060708090A0B0C0D0E0F
    // Data to write
    std::vector<uint8_t> writedata(16, 'A');
    auto cmd = std::dynamic_pointer_cast<logicalaccess::MifarePlusSL3Commands_NEW>(chip->getCommands());
    LLA_ASSERT(cmd, "Invalid command object.");
    cmd->resetAuth();
    LLA_ASSERT(cmd->authenticate(0, aes_key, logicalaccess::MifareKeyType::KT_KEY_A),
               "Failed to perform auth");
    cmd->readBinaryPlain(9, 16);

    // Data read
    std::vector<uint8_t> readdata;

    pcsc_test_shutdown(readerUnit);
    return 0;
}
