#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "lla-tests/macros.hpp"
#include "lla-tests/utils.hpp"
#include "logicalaccess/logs.hpp"
#include <logicalaccess/services/identity/identity_service.hpp>
#include <logicalaccess/crypto/sha.hpp>
#include <pluginscards/epass/epassaccessinfo.hpp>
#include <ctime>

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
    tie(provider, readerUnit, chip) = lla_test_init("EPass");

    PRINT_TIME("Chip identifier: " <<
                   logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    LLA_ASSERT(chip->getCardType() == "EPass",
               "Chip is not a EPass, but is " + chip->getCardType() + " instead.");

    auto srv = std::dynamic_pointer_cast<IdentityCardService>(chip->getService(CST_IDENTITY));
    LLA_ASSERT(srv, "Cannot retrieve identity service from the chip");
    // Prepare the service.
    auto ai = std::make_shared<EPassAccessInfo>();
    ai->mrz_ = "W7GCH9ZY24UTO7904107F2006187<<<<<<<<<<<<<<<2";
    srv->setAccessInfo(ai);


    std::string name = srv->getString(IdentityCardService::MetaData::NAME);
    LLA_ASSERT("ANDERSON  JANE" == name, "Name doesn't match.");
    LLA_SUBTEST_PASSED("GetName");

    ByteVector picture_data = srv->getData(IdentityCardService::MetaData::PICTURE);

    // We check the hash of the picture rather than the full picture bytes. Easier for tests.
    LLA_ASSERT(openssl::SHA1Hash(picture_data) == BufferHelper::fromHexString("9cb474bfb578a9c8defa8eb6fe9ea2cd643be308"),
               "Retrieved image picture doesn't match expected picture.");
    LLA_SUBTEST_PASSED("GetPicture");

	std::string nationality = srv->getString(IdentityCardService::MetaData::NATIONALITY);
    LLA_ASSERT("UTO" == nationality, "Nationality doesn't match.");
    LLA_SUBTEST_PASSED("GetNationality");


	std::string docno = srv->getString(IdentityCardService::MetaData::DOC_NO);
    LLA_ASSERT(docno == "W7GCH9ZY2", "Document number doesn't match.");
    LLA_SUBTEST_PASSED("GetDocNo");


	std::chrono::system_clock::time_point tp = srv->getTime(IdentityCardService::MetaData::BIRTHDATE);
    time_t tp_t = std::chrono::system_clock::to_time_t(tp);
    tm tm = *localtime(&tp_t);

    char buff[512];
    strftime(buff, sizeof(buff), "%c", &tm);
    PRINT_TIME("Birthdate: " << buff);

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
