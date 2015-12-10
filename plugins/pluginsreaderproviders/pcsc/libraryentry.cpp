#include <string>
#include <memory>
#include <logicalaccess/utils.hpp>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "pcscreaderprovider.hpp"
#include "readers/omnikeylanxx21readerunit.hpp"
#include "readers/omnikeyxx25readerunit.hpp"
#include "readers/scmreaderunit.hpp"
#include "readers/springcardreaderunit.hpp"
#include "readers/cherryreaderunit.hpp"
#include "readers/acsacrreaderunit.hpp"
#include "readers/acsacr1222lreaderunit.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C" {

#define REGISTER_READER_USB(factory_helper, type, vendor_id, product_id)            \
    (factory_helper)                                                                \
        .registerReader((vendor_id), (product_id),                                  \
                        std::bind(&std::make_shared<type, const std::string &>,     \
                                  std::placeholders::_1));

#define REGISTER_READER(factory_helper, type, regexp)                               \
    (factory_helper)                                                                \
        .registerReader((regexp),                                                   \
                        std::bind(&std::make_shared<type, const std::string &>,     \
                                  std::placeholders::_1));


/**
* Attempt to create a ReaderUnit object for a reader with name readName.
* This function returns NULL if there is no match, is does NOT create a default
* PCSCReaderUnit.
*/
LIBLOGICALACCESS_API void
getReaderUnit(const std::string &readerIdentifier, const std::string &readerName,
              std::shared_ptr<logicalaccess::ReaderUnit> &u)
{
    using namespace std;
    using namespace logicalaccess;

    ReaderFactoryHelper rfh;

    REGISTER_READER_USB(rfh, SpringCardReaderUnit, 0x1C34, 0x8141);
    REGISTER_READER(rfh, SpringCardReaderUnit, ".*SpringCard.*");

    REGISTER_READER(rfh, OmnikeyLANXX21ReaderUnit,
                    ".*OMNIKEY.*(x21|5321|5321).*(LAN).*");
    REGISTER_READER(rfh, OmnikeyXX21ReaderUnit,
                    ".*OMNIKEY.*(LAN).*(x21|5321|5321).*");
    REGISTER_READER(rfh, OmnikeyXX21ReaderUnit, ".*OMNIKEY.*(x21|5321|5321).*");

    REGISTER_READER(rfh, OmnikeyXX25ReaderUnit, ".*OMNIKEY.*(x25|5025-CL).*");

    REGISTER_READER(rfh, SCMReaderUnit, ".*SDI010 Contactless Reader.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*SCR331-DI USB ContactlessReader.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*SCL010 Contactless.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*SCL01x Contactless.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*SCL3711 reader.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*Identive CLOUD 4700 F Contactless.*");
    REGISTER_READER(rfh, SCMReaderUnit, ".*Identive CLOUD 4710 F Contactless.*");
    REGISTER_READER(rfh, SCMReaderUnit,
                    ".*SCM Microsystems Inc. SCL011G Contactless Reader.*");

    REGISTER_READER(rfh, CherryReaderUnit, ".*Cherry .*");

    REGISTER_READER(rfh, ACSACR1222LReaderUnit,
                    ".*ACS ACR1222 3S PICC Reader PICC.*");
    REGISTER_READER(rfh, ACSACR1222LReaderUnit,
                    ".*ACS ACR1222 3S PICC Reader 00 00.*");

    REGISTER_READER(rfh, ACSACRReaderUnit, ".*ACS ACR.*");

    u = rfh.instanciate(readerIdentifier, readerName);
}

LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"PCSC";
}

LIBLOGICALACCESS_API void
getPCSCReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != NULL)
    {
        *rp = logicalaccess::PCSCReaderProvider::createInstance();
    }
}

LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char *readername,
                                          size_t readernamelen, void **getterfct)
{
    bool ret = false;
    if (readername != NULL && readernamelen == PLUGINOBJECT_MAXLEN &&
        getterfct != NULL)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getPCSCReader;
            sprintf(readername, READER_PCSC);
            ret = true;
        }
        break;
        }
    }

    return ret;
}
}
