#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/promag/promagreaderprovider.hpp>

extern "C" {
LLA_READERS_PROMAG_API char *getLibraryName()
{
    return (char *)"Promag";
}

LLA_READERS_PROMAG_API void
getPromagReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::PromagReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_PROMAG_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getPromagReader;
            sprintf(readername, READER_PROMAG);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}