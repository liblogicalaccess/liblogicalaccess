#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "pluginscards/mifare/mifarechip.hpp"
#include "pluginscards/mifare/mifarelocation.hpp"

#include "logicalaccess/logs.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target Mifare1K cards. Test the access storage service");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("WriteService");
    LLA_SUBTEST_REGISTER("ReadService");
    LLA_SUBTEST_REGISTER("CorrectWriteRead");
}

int main(int ac, char **av)
{
    using namespace logicalaccess;

    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("Chip identifier: " <<
               logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "Mifare1K",
               "Chip is not a Mifare1K, but is " + chip->getCardType() +
               " instead.");

    auto storage = std::dynamic_pointer_cast<logicalaccess::StorageCardService>(
            chip->getService(logicalaccess::CST_STORAGE));

    std::shared_ptr<logicalaccess::Location> location;
    std::shared_ptr<logicalaccess::AccessInfo> aiToUse;
    std::shared_ptr<logicalaccess::AccessInfo> aiToWrite;

    // We want to write data on sector 7.
    std::shared_ptr<logicalaccess::MifareLocation> mlocation(
            new logicalaccess::MifareLocation());
    mlocation->sector = 7;
    mlocation->block = 0;
    location = mlocation;

    // Key to use for sector authentication
    std::shared_ptr<logicalaccess::MifareAccessInfo> maiToUse(
            new logicalaccess::MifareAccessInfo());
    maiToUse->keyA->fromString("ff ff ff ff ff ff");      // Default key
    maiToUse->keyB->fromString("ff ff ff ff ff ff");
    aiToUse = maiToUse;

    // Change the sector key with the following key
    std::shared_ptr<logicalaccess::MifareAccessInfo> maiToWrite(
            new logicalaccess::MifareAccessInfo());
    maiToWrite->keyA->fromString("ff ff ff ff ff ff");
    maiToWrite->keyB->fromString("ff ff ff ff ff ff");
    aiToWrite = maiToWrite;

    // Data to write
    std::vector<uint8_t> writedata(16, 'e');

    // Data read
    std::vector<uint8_t> readdata;

    // Write data on the specified location with the specified key
    storage->writeData(location, aiToUse, aiToWrite, writedata, logicalaccess::CB_DEFAULT);
    using namespace logicalaccess; // required for overload of std::ostream(vector &)
    PRINT_TIME("Wrote: " << writedata);
    LLA_SUBTEST_PASSED("WriteService")

    const int mifare_block_size = 16;
    // We read the data on the same location. Remember, the key is now changed.
    readdata = storage
            ->readData(location, aiToWrite, mifare_block_size, logicalaccess::CB_DEFAULT);
    PRINT_TIME("Read: " << readdata);
    LLA_SUBTEST_PASSED("ReadService")

    LLA_ASSERT(std::equal(writedata.begin(), writedata.end(), readdata.begin()),
               "Data read is not what we wrote.");
    LLA_SUBTEST_PASSED("CorrectWriteRead");

    pcsc_test_shutdown(readerUnit);
    return 0;
}
