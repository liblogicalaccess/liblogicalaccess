#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "gunneboreaderprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C" {
LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"Gunnebo";
}

LIBLOGICALACCESS_API void
getGunneboReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::GunneboReaderProvider::getSingletonInstance();
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
            *getterfct = (void *)&getGunneboReader;
            sprintf(readername, READER_GUNNEBO);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}