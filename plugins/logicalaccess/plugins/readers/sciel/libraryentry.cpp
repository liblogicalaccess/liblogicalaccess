#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/sciel/scielreaderprovider.hpp>

extern "C" {
LLA_READERS_SCIEL_API char *getLibraryName()
{
    return (char *)"SCIEL";
}

LLA_READERS_SCIEL_API void
getSCIELReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::SCIELReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_SCIEL_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getSCIELReader;
            sprintf(readername, READER_SCIEL);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}