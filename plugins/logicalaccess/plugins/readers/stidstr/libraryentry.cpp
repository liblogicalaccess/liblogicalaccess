#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstrreaderprovider.hpp>

extern "C" {
LLA_READERS_STIDSTR_API char *getLibraryName()
{
    return (char *)"STidSTR";
}

LLA_READERS_STIDSTR_API void
getSTidSTRReader(std::shared_ptr<logicalaccess::STidSTRReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::STidSTRReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_STIDSTR_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getSTidSTRReader;
            sprintf(readername, READER_STIDSTR);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}