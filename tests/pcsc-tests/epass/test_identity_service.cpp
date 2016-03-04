#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"
#include "logicalaccess/logs.hpp"
#include <logicalaccess/services/identity/identity_service.hpp>
#include <logicalaccess/crypto/sha.hpp>
#include <pluginscards/epass/epass_access_info.hpp>

void introduction()
{
    PRINT_TIME("This test target cards that can hold an identity. Mostly EPassport.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("GetName");
    LLA_SUBTEST_REGISTER("GetPicture");
    LLA_SUBTEST_REGISTER("GetNationality");
    LLA_SUBTEST_REGISTER("GetDocNo");
}

using namespace logicalaccess;

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    ReaderProviderPtr provider;
    ReaderUnitPtr readerUnit;
    ChipPtr chip;
    std::tie(provider, readerUnit, chip) = lla_test_init("EPass");

    PRINT_TIME("CHip identifier: " <<
                   logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "EPass",
               "Chip is not a EPass, but is " + chip->getCardType() + " instead.");

    auto srv = std::dynamic_pointer_cast<IdentityCardService>(chip->getService(CST_IDENTITY));
    LLA_ASSERT(srv, "Cannot retrieve identity service from the chip");
    // Prepare the service.
    auto ai = std::make_shared<EPassAccessInfo>();
    ai->mrz_ = "W7GCH9ZY24UTO7904107F2006187<<<<<<<<<<<<<<<2";
    srv->setAccessInfo(ai);


    std::string name;
    LLA_ASSERT(srv->get(IdentityCardService::MetaData::NAME, name), "Failed to fetch name");
    LLA_ASSERT("ANDERSON  JANE" == name, "Name doesn't match.");
    LLA_SUBTEST_PASSED("GetName");

    ByteVector picture_data;
    LLA_ASSERT(srv->get(IdentityCardService::MetaData::PICTURE, picture_data), "Failed to"
        "get picture bytes");

    // We check the hash of the picture rather than the full picture bytes. Easier for tests.
    LLA_ASSERT(openssl::SHA1Hash(picture_data) == BufferHelper::fromHexString("9cb474bfb578a9c8defa8eb6fe9ea2cd643be308"),
               "Retrieved image picture doesn't match expected picture.");
    LLA_SUBTEST_PASSED("GetPicture");

    std::string nationality;
    LLA_ASSERT(srv->get(IdentityCardService::MetaData::NATIONALITY, nationality),
               "Failed to fetch nationality");
    LLA_ASSERT("UTO" == nationality, "Name doesn't match.");
    LLA_SUBTEST_PASSED("GetNationality");


    std::string docno;
    LLA_ASSERT(srv->get(IdentityCardService::MetaData::DOC_NO, docno),
               "Failed to fetch document number.");
    LLA_ASSERT(docno == "W7GCH9ZY2", "Document number doesn't match.");
    LLA_SUBTEST_PASSED("GetDocNo");


    std::chrono::system_clock::time_point tp;
    LLA_ASSERT(srv->get(IdentityCardService::MetaData::BIRTHDATE, tp), "Failed to "
        "fetch birthdate");
    std::time_t tp_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tp_t);
    PRINT_TIME("Birthdate: " << std::put_time(&tm, "%c"));

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
