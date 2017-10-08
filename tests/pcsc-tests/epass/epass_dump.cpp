#include "logicalaccess/dynlibrary/idynlibrary.hpp"

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
    PRINT_TIME("Dump some information about an EPassport.");

    PRINT_TIME("You will have 20 seconds to insert a card.");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
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
    std::string mrz;
    std::cout << "Enter MRZ please: ";
    std::cin >> ai->mrz_;
    srv->setAccessInfo(ai);

	std::string name = srv->getString(IdentityCardService::MetaData::NAME);
    PRINT_TIME("Name: " + name);

	std::string nationality = srv->getString(IdentityCardService::MetaData::NATIONALITY);
    PRINT_TIME("Country: " + nationality);

	std::string docno = srv->getString(IdentityCardService::MetaData::DOC_NO);
    PRINT_TIME("Docno: " + docno);

	std::chrono::system_clock::time_point tp = srv->getTime(IdentityCardService::MetaData::BIRTHDATE);
    time_t tp_t = std::chrono::system_clock::to_time_t(tp);
    tm tm = *localtime(&tp_t);

    char buff[512];
    strftime(buff, sizeof(buff), "%c", &tm);
    PRINT_TIME("Birthdate: " << buff);

	ByteVector picture_data = srv->getData(IdentityCardService::MetaData::PICTURE);
    {
        std::ofstream of("/tmp/passport_pic.jpeg");
        of.write((const char *)picture_data.data(), picture_data.size());
    }
    PRINT_TIME("Photo was dumped into: /tmp/passport_pic.jpeg");

    pcsc_test_shutdown(readerUnit);

    return EXIT_SUCCESS;
}
