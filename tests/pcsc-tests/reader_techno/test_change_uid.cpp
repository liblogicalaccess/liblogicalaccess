#include <chrono>
#include <thread>
#include <logicalaccess/crypto/lla_random.hpp>

#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/services/uidchanger/uidchangerservice.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"

using namespace logicalaccess;

void introduction()
{
    PRINT_TIME("Test that we can change the UID of some card. This test targets Mifare Ultralight");

    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("ChangeCardTechno");
}

ByteVector gen_random_uid()
{
    return RandomHelper::bytes(7);
}

void change_uid(const ByteVector &new_uid)
{
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("Chip identifier: " <<
                   BufferHelper::getHex(chip->getChipIdentifier()));
    PRINT_TIME("Card type: {" << chip->getCardType() << "}");

    std::shared_ptr<UIDChangerService> uid_changer = std::dynamic_pointer_cast<UIDChangerService>(
        chip->getService(CST_UID_CHANGER));

    LLA_ASSERT(uid_changer, "No UID Changer service available for this card.");

    PRINT_TIME("Changing UID to " << BufferHelper::getHex(new_uid));
    uid_changer->changeUID(new_uid);
    pcsc_test_shutdown(readerUnit);
}

void check_uid(const ByteVector &new_uid)
{
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = lla_test_init();

    LLA_ASSERT(chip->getChipIdentifier() == new_uid, "UID was not changed.");

    pcsc_test_shutdown(readerUnit);
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();

    auto new_uid = gen_random_uid();
    change_uid(new_uid);
    PRINT_TIME("Now insert the card again in 2seconds, to check if the UID was properly changed");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    check_uid(new_uid);
    return 0;
}
