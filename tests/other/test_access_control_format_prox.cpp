#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include <pluginscards/prox/proxlocation.hpp>

#include "pluginscards/desfire/desfireev1chip.hpp"
#include "pluginscards/desfire/desfireev1commands.hpp"
#include "pluginsreaderproviders/iso7816/commands/desfireev1iso7816commands.hpp"


#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

void introduction()
{
    PRINT_TIME("This test attempt to read access control bits from a card.");

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
    tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME(
        "Chip identifier: " << BufferHelper::getHex(chip->getChipIdentifier()));

    std::shared_ptr<AccessControlCardService> acs =
        std::dynamic_pointer_cast<AccessControlCardService>(
            chip->getService(CST_ACCESS_CONTROL));


    auto ret = acs->readFormat(std::make_shared<Wiegand26Format>(),
                               std::make_shared<ProxLocation>(), nullptr);
    auto buffer = ret->getLinearData();

    std::cout << "Format: " << ByteVector(buffer.begin(), buffer.end())
              << std::endl;

    pcsc_test_shutdown(readerUnit);
    return 0;
}
