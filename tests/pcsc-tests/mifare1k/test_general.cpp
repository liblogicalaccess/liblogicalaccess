#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"

#include "pluginscards/mifare/mifarecommands.hpp"
#include "pluginscards/mifare/mifarelocation.hpp"
#include "pluginsreaderproviders/pcsc/readers/cardprobes/cl1356cardprobe.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"


void introduction()
{
    PRINT_TIME("This test target Mifare1K cards.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("LoadKey");
    LLA_SUBTEST_REGISTER("Authenticate");
    LLA_SUBTEST_REGISTER("WriteReadBinary");
    LLA_SUBTEST_REGISTER("WriteReadSector");
    LLA_SUBTEST_REGISTER("WriteReadSectors");
    LLA_SUBTEST_REGISTER("ChangeKey");
    LLA_SUBTEST_REGISTER("WriteReadFormat");
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

    PRINT_TIME("Chip identifier: " <<
		logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "Mifare1K",
               "Chip is not a Mifare1K, but is " + chip->getCardType() +
               " instead.");

    auto cmd = std::dynamic_pointer_cast<MifareCommands>(chip->getCommands());

    auto key = std::make_shared<MifareKey>("ff ff ff ff ff ff");
    cmd->loadKey(0, KT_KEY_A, key);
    LLA_SUBTEST_PASSED("LoadKey");

    cmd->authenticate(8, 0, KT_KEY_A);
    LLA_SUBTEST_PASSED("Authenticate");

    ByteVector data(16), tmp;
    data[0] = 0x11;
    data[15] = 0xff;
    cmd->updateBinary(8, data);

    tmp = cmd->readBinary(8, 16);

    LLA_ASSERT(std::equal(data.begin(), data.end(), tmp.begin()),
               "read and write data are different!");
    LLA_SUBTEST_PASSED("WriteReadBinary");

    data.clear();
    data.resize(48);
    data[0] = 0x11;
    data[47] = 0xff;
    MifareAccessInfo::SectorAccessBits sab;

	cmd->writeSector(2, 0, data, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), sab, CB_DEFAULT, &sab);

	tmp = cmd->readSector(2, 0, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), sab);
    LLA_ASSERT(std::equal(data.begin(), data.end(), tmp.begin()),
               "read and write data are different!");
    LLA_SUBTEST_PASSED("WriteReadSector");

    data.clear();
    data.resize(96);
    data[0] = 0x11;
    data[95] = 0xff;

	cmd->writeSectors(2, 3, 0, data, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), sab);

	tmp = cmd->readSectors(2, 3, 0, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), sab);
    LLA_ASSERT(std::equal(data.begin(), data.end(), tmp.begin()),
               "read and write data are different!");
    LLA_SUBTEST_PASSED("WriteReadSectors");


    auto newkey = std::make_shared<MifareKey>("ff ff ff ff ff fa");

	cmd->changeKeys(KT_KEY_A, std::shared_ptr<MifareKey>(), newkey, std::shared_ptr<MifareKey>(), 2, &sab);
	cmd->changeKeys(KT_KEY_A, newkey, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), 2, &sab);
    LLA_SUBTEST_PASSED("ChangeKey");


    auto service = std::dynamic_pointer_cast<AccessControlCardService>(
            chip->getService(CST_ACCESS_CONTROL));

    LLA_ASSERT(service, "Cannot retrieve service");
    auto location = std::make_shared<MifareLocation>();
    location->sector = 2;
    auto format = std::make_shared<Wiegand26Format>();
    format->setUid(1000);
    format->setFacilityCode(67);

    service->writeFormat(format, location, std::shared_ptr<AccessInfo>(),
                         std::shared_ptr<AccessInfo>());

    auto formattmp = std::make_shared<Wiegand26Format>();
    auto rformat = std::dynamic_pointer_cast<Wiegand26Format>(
            service->readFormat(formattmp, location,
                                std::shared_ptr<AccessInfo>()));

    if (!rformat || rformat->getUid() != 1000 || rformat->getFacilityCode() != 67)
    THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Bad format");
    LLA_SUBTEST_PASSED("WriteReadFormat");

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
