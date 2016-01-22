#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "pluginscards/mifare/mifarechip.hpp"

#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"
#include "logicalaccess/cards/locationnode.hpp"

void introduction()
{
    PRINT_TIME("This test target Mifare1K cards. Iterate over memory area and count them.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("Iteration");
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
               BufferHelper::getHex(chip->getChipIdentifier()));
/*
    LLA_ASSERT(chip->getCardType() == "Mifare1K",
               "Chip is not a Mifare1K, but is " + chip->getCardType() +
               " instead.");*/


    // Get the root node
    std::shared_ptr<LocationNode> rootNode = chip->getRootLocationNode();
    // Get childrens of this node
    std::vector<std::shared_ptr<LocationNode> > childs = rootNode->getChildrens();
    int size = 0 ;
    for (std::vector<std::shared_ptr<LocationNode> >::iterator i = childs.begin(); i != childs.end(); ++i)
    {
        // Display node information
        int tmp_size = (*i)->getLength() * (*i)->getUnit();
        std::cout << "Size of node " << (*i)->getName() << ": " << tmp_size << " bytes" << std::endl;
        size += tmp_size;
    }
    LLA_ASSERT(childs.size() == 16, "Unexpected number of children");
    LLA_ASSERT(size == 768, "Memory size unexpected");
    LLA_SUBTEST_PASSED("Iteration");

    pcsc_test_shutdown(readerUnit);
    return 0;
}
