#include <pluginscards/desfire/nxpav2keydiversification.hpp>
#include "logicalaccess/cards/IKSStorage.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

#include "pluginsreaderproviders/iso7816/commands/desfireev1iso7816commands.hpp"
#include "pluginscards/desfire/desfireev1commands.hpp"
#include "pluginscards/desfire/desfireev1chip.hpp"
#include "pluginscards/desfire/nxpav1keydiversification.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test target DESFireEV1 cards. It tests that we are "
               "able to change a key using Islog Key Server.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("WriteService");
    LLA_SUBTEST_REGISTER("ReadService");
    LLA_SUBTEST_REGISTER("CorrectWriteRead");
}

std::vector<uint8_t> vector_from_string(const std::string &s)
{
    std::vector<uint8_t> ret(s.begin(), s.end());
    return ret;
}

void read_write(std::shared_ptr<logicalaccess::DESFireEV1ISO7816Commands> cmdev1)
{
    using namespace logicalaccess;
    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_AES);

    key->setKeyStorage(
        std::make_shared<logicalaccess::IKSStorage>("imported-one-aes"));
    auto div = std::make_shared<NXPAV2KeyDiversification>();
    div->setSystemIdentifier(vector_from_string("BOAP"));
    key->setKeyDiversification(div);

    static_cast<DESFireISO7816Commands *>(cmdev1.get())->selectApplication(0x535);
    cmdev1->authenticate(0, key);

    // The excepted memory tree
    std::shared_ptr<logicalaccess::DESFireEV1Location> dlocation(
        new logicalaccess::DESFireEV1Location());

    auto storage = std::dynamic_pointer_cast<logicalaccess::StorageCardService>(
        cmdev1->getChip()->getService(logicalaccess::CST_STORAGE));

    // The Application ID to use
    dlocation->aid = 0x535;
    // File 0 into this application
    dlocation->file = 0;
    // File communication requires encryption
    dlocation->securityLevel = logicalaccess::CM_ENCRYPT;
    dlocation->useEV1        = true;
    dlocation->cryptoMethod  = logicalaccess::DF_KEY_AES;

    std::shared_ptr<logicalaccess::DESFireAccessInfo> daiToUse(
        new logicalaccess::DESFireAccessInfo());
    daiToUse->masterApplicationKey = key;
    daiToUse->writeKeyno           = 0;
    daiToUse->writeKey             = key;
    daiToUse->readKey              = key;
    daiToUse->readKeyno            = 0;

    // Data to write
    std::vector<uint8_t> writedata(16, 'd');
    std::vector<uint8_t> readdata;
    // Write data on the specified location with the specified key
    storage->writeData(dlocation, daiToUse, daiToUse, writedata,
                       logicalaccess::CB_DEFAULT);

    PRINT_TIME("Wrote: " << writedata);
    LLA_SUBTEST_PASSED("WriteService")

    // We read the data on the same location. Remember, the key is now changed.
    readdata = storage->readData(dlocation, daiToUse, 16, logicalaccess::CB_DEFAULT);
    PRINT_TIME("Read: " << readdata);
    LLA_SUBTEST_PASSED("ReadService")

    LLA_ASSERT(std::equal(writedata.begin(), writedata.end(), readdata.begin()),
               "Data read is not what we wrote.");
    LLA_SUBTEST_PASSED("CorrectWriteRead");
}

void create_app_and_file(
    std::shared_ptr<logicalaccess::DESFireISO7816Commands> cmd,
    std::shared_ptr<logicalaccess::DESFireEV1ISO7816Commands> cmdev1)
{
    // create the application we wish to write into
    cmdev1->createApplication(0x535, logicalaccess::DESFireKeySettings::KS_DEFAULT,
                              3, logicalaccess::DESFireKeyType::DF_KEY_AES,
                              logicalaccess::FIDS_NO_ISO_FID, 0,
                              std::vector<unsigned char>());
    cmd->selectApplication(0x535);

    std::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
    key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_AES);
    key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    std::vector<uint8_t> bla(key->getData(), key->getData() + key->getLength());

    using namespace logicalaccess;
    key->setKeyStorage(
        std::make_shared<logicalaccess::IKSStorage>("imported-zero-aes"));
    cmd->authenticate(0, key);

    std::shared_ptr<logicalaccess::DESFireKey> new_key(
        new logicalaccess::DESFireKey());
    new_key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_AES);
    auto div = std::make_shared<NXPAV2KeyDiversification>();
    div->setSystemIdentifier({'B', 'O', 'A', 'P'});
    new_key->setKeyDiversification(div);
    new_key->setKeyStorage(
        std::make_shared<logicalaccess::IKSStorage>("imported-one-aes"));

    // We can do everything with key1
    logicalaccess::DESFireAccessRights ar;
    ar.readAccess         = logicalaccess::TaskAccessRights::AR_KEY0;
    ar.writeAccess        = logicalaccess::TaskAccessRights::AR_KEY0;
    ar.readAndWriteAccess = logicalaccess::TaskAccessRights::AR_KEY0;
    ar.changeAccess       = logicalaccess::TaskAccessRights::AR_KEY0;

    // Create the file we will use.
    int file_size = 16;
    cmdev1->createStdDataFile(0x00, logicalaccess::EncryptionMode::CM_ENCRYPT, ar,
                              file_size, 0);
    cmd->authenticate(0, key);

    std::shared_ptr<DESFireProfile> dprofile =
        std::dynamic_pointer_cast<DESFireProfile>(cmd->getChip()->getProfile());
    assert(dprofile);
    dprofile->setKey(0x535, 0, key);

    cmd->changeKey(0, new_key);
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

    PRINT_TIME("Chip identifier: "
               << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "DESFireEV1",
               "Chip is not an DESFireEV1, but is " + chip->getCardType() +
                   " instead.");

    auto storage = std::dynamic_pointer_cast<logicalaccess::StorageCardService>(
        chip->getService(logicalaccess::CST_STORAGE));

    auto cmd = std::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(
        chip->getCommands());
    std::shared_ptr<logicalaccess::DESFireEV1ISO7816Commands> cmdev1 =
        std::dynamic_pointer_cast<logicalaccess::DESFireEV1ISO7816Commands>(
            chip->getCommands());

    std::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
    key->setKeyType(logicalaccess::DESFireKeyType::DF_KEY_DES);
    key->setKeyStorage(
        std::make_shared<logicalaccess::IKSStorage>("imported-zero-des"));
    cmd->selectApplication(0x00);
    cmd->authenticate(0, key);
    /// 1 command line parameter will format card
    /// 2 will create application/file
    /// 3 will write/read the card.
    if (ac == 3)
        create_app_and_file(cmd, cmdev1);
    else if (ac == 4)
    {
        read_write(cmdev1);
    }
    else if (ac == 2)
    {
        cmd->selectApplication(0x00);
        cmd->authenticate(0, key);
        cmd->deleteApplication(0x535);
        cmd->erase();
    }
    return 0;
}
