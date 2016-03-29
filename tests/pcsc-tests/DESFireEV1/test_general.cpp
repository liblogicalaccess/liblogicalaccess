#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "pluginscards/desfire/desfireev1chip.hpp"
#include "pluginsreaderproviders/iso7816/commands/desfireev1iso7816commands.hpp"
#include "pluginscards/desfire/desfirecommands.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target DESFireEV1 cards.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("Authenticate");
    LLA_SUBTEST_REGISTER("ChangeKey");
    LLA_SUBTEST_REGISTER("WriteRead");
    LLA_SUBTEST_REGISTER("ReadFormat");
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

    PRINT_TIME("CHip identifier: " <<
               logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "DESFireEV1",
               "Chip is not an DESFireEV1, but is " + chip->getCardType() +
               " instead.");

    std::shared_ptr<DESFireEV1Chip> desfirechip = std::dynamic_pointer_cast<DESFireEV1Chip>(chip);
    assert(desfirechip);
    PRINT_TIME("Has Real UID: " << desfirechip->hasRealUID());

    auto location_root_node = chip->getRootLocationNode();

    auto cmd = std::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(
            chip->getCommands());
    auto cmdev1 = std::dynamic_pointer_cast<logicalaccess::DESFireEV1ISO7816Commands>(
            chip->getCommands());
    LLA_ASSERT(cmd && cmdev1, "Cannot get correct command object from chip.");

    cmd->selectApplication(0x00);
    cmd->authenticate(0);

    cmd->erase();
    cmdev1->createApplication(0x521, logicalaccess::DESFireKeySettings::KS_DEFAULT, 3,
                              logicalaccess::DESFireKeyType::DF_KEY_AES,
                              logicalaccess::FIDS_NO_ISO_FID, 0, std::vector<unsigned char>());

    cmd->selectApplication(0x521);
    std::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
    key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_AES);
    cmd->authenticate(0, key);
    LLA_SUBTEST_PASSED("Authenticate");

    logicalaccess::DESFireAccessRights ar;
    ar.readAccess = logicalaccess::TaskAccessRights::AR_KEY2;
    ar.writeAccess = logicalaccess::TaskAccessRights::AR_KEY1;
    ar.readAndWriteAccess = logicalaccess::TaskAccessRights::AR_KEY1;
    ar.changeAccess = logicalaccess::TaskAccessRights::AR_KEY1;
    cmdev1->createStdDataFile(0x00, logicalaccess::EncryptionMode::CM_ENCRYPT, ar, 4, 0);


    cmd->authenticate(1, key);
    std::vector<unsigned char> data = {0x01, 0x02, 0x03, 0x04}, tmp;
    cmdev1->writeData(0, 0, data, logicalaccess::EncryptionMode::CM_ENCRYPT);

    cmd->authenticate(2, key);
    tmp = cmdev1->readData(0, 0, 4, logicalaccess::EncryptionMode::CM_ENCRYPT);
    LLA_ASSERT(std::equal(data.begin(), data.end(), tmp.begin()),
               "read and write data are different!");
    LLA_SUBTEST_PASSED("WriteRead");

    cmd->authenticate(0x00, key);
    cmd->deleteFile(0x00);

    cmd->authenticate(0x00, key);
    std::shared_ptr<logicalaccess::DESFireKey> newkey(
            new logicalaccess::DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03"));
    cmd->changeKey(0x00, newkey);
    LLA_SUBTEST_PASSED("ChangeKey");

    cmd->selectApplication(0x00);
    cmd->authenticate(0);
    cmd->deleteApplication(0x521);

    auto service = std::dynamic_pointer_cast<logicalaccess::AccessControlCardService>(
            chip->getService(logicalaccess::CardServiceType::CST_ACCESS_CONTROL));
    LLA_ASSERT(service, "Cannot retrieve access control service from chip.");

    auto location = std::make_shared<logicalaccess::DESFireLocation>();
    location->aid = 0x522;
    location->file = 0;
    auto ai = std::make_shared<logicalaccess::DESFireAccessInfo>();
    auto format = std::make_shared<logicalaccess::Wiegand26Format>();
    format->setUid(1000);
    format->setFacilityCode(67);

    service->writeFormat(format, location, ai, ai);
    auto formattmp = std::make_shared<logicalaccess::Wiegand26Format>();
    auto rformat = std::dynamic_pointer_cast<logicalaccess::Wiegand26Format>(
            service->readFormat(formattmp, location, ai));

    if (!rformat || rformat->getUid() != 1000 || rformat->getFacilityCode() != 67)
    THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Bad format");
    LLA_SUBTEST_PASSED("ReadFormat");

    PRINT_TIME("Has Real UID: " << desfirechip->hasRealUID());
    PRINT_TIME("Chip UID" << chip->getChipIdentifier());

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
