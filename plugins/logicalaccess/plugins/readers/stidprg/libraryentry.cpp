#include <logicalaccess/plugins/readers/stidprg/STidPRG_Prox_AccessControl.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgreaderprovider.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgreaderunit.hpp>
#include <cassert>

extern "C" {
LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"STidPRG";
}

LIBLOGICALACCESS_API void
getSTidPRGReader(std::shared_ptr<logicalaccess::STidPRGReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::STidPRGReaderProvider::getSingletonInstance();
    }
}

LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char *readername,
                                          size_t readernamelen, void **getterfct)
{
    bool ret = false;
    if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN &&
        getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getSTidPRGReader;
            sprintf(readername, "STidPRG");
            ret = true;
        }
        break;
        default:;
        }
    }
    return ret;
}

LIBLOGICALACCESS_API void
getCardService(std::shared_ptr<logicalaccess::Chip> c,
               std::shared_ptr<logicalaccess::CardService> &service,
               logicalaccess::CardServiceType type)
{
    // We want to instanciate AccessControl in order to write them
    // onto the card. This works if the card type is Prox, and the
    // reader is a STidPRG.

    if (type != logicalaccess::CST_ACCESS_CONTROL)
        return;

    // We must first fetch the reader unit and make sure its a STidPRG,
    // otherwise we cannot create the Service.
    if (!c || !c->getCommands() || !c->getCommands()->getReaderCardAdapter() ||
        !c->getCommands()->getReaderCardAdapter()->getDataTransport() ||
        !c->getCommands()->getReaderCardAdapter()->getDataTransport()->getReaderUnit())
    {
        return;
    }
    std::shared_ptr<logicalaccess::ReaderUnit> ru =
        c->getCommands()->getReaderCardAdapter()->getDataTransport()->getReaderUnit();
    if (c->getCardType() == "Prox" &&
        std::dynamic_pointer_cast<logicalaccess::STidPRGReaderUnit>(ru))
    {
        service = std::make_shared<logicalaccess::STidPRGProxAccessControl>(c);
    }
}
}
