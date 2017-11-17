#include <chrono>
#include <thread>
#include <logicalaccess/plugins/crypto/lla_random.hpp>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/services/uidchanger/uidchangerservice.hpp>
#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>

using namespace logicalaccess;

void introduction()
{
    PRINT_TIME("Test that we can change the UID of some card."
               " This test targets Mifare Ultralight and Mifare Classic");

    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("ChangeUID");
    LLA_SUBTEST_REGISTER("ValidateUIDChange");
}

ByteVector gen_random_uid()
{
    return RandomHelper::bytes(7);
}

void change_uid(ByteVector new_uid)
{
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init();

    PRINT_TIME("Chip identifier: " << BufferHelper::getHex(chip->getChipIdentifier()));
    PRINT_TIME("Card type: {" << chip->getCardType() << "}");

    std::shared_ptr<UIDChangerCardService> uid_changer =
        std::dynamic_pointer_cast<UIDChangerCardService>(chip->getService(CST_UID_CHANGER));

    LLA_ASSERT(uid_changer, "No UID Changer service available for this card.");

    if (chip->getCardType() == "Mifare1K")
    {
        // For MifareClassic the UID is 4bytes.
        new_uid = ByteVector(new_uid.begin(), new_uid.begin() + 4);
    }

    PRINT_TIME("Changing UID to " << BufferHelper::getHex(new_uid));
    uid_changer->changeUID(new_uid);
    LLA_SUBTEST_PASSED("ChangeUID");
    pcsc_test_shutdown(readerUnit);
}

void check_uid(ByteVector new_uid)
{
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init();

    if (chip->getCardType() == "Mifare1K")
    {
        // For MifareClassic the UID is 4bytes.
        new_uid = ByteVector(new_uid.begin(), new_uid.begin() + 4);
    }

    LLA_ASSERT(chip->getChipIdentifier() == new_uid, "UID was not changed.");
    LLA_SUBTEST_PASSED("ValidateUIDChange");
    pcsc_test_shutdown(readerUnit);
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();

    auto new_uid = gen_random_uid();
    change_uid(new_uid);
    PRINT_TIME("Now insert the card again in 2seconds, to check if the UID was properly "
               "changed");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    check_uid(new_uid);
    return 0;
}
