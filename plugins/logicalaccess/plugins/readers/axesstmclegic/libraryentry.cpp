#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/axesstmclegic/axesstmclegicreaderprovider.hpp>

extern "C" {
LLA_READERS_AXESSTMCLEGIC_API char *getLibraryName()
{
    return (char *)"AxessTMCLegic";
}

LLA_READERS_AXESSTMCLEGIC_API void
getAxessTMCLegicReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::AxessTMCLegicReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_AXESSTMCLEGIC_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getAxessTMCLegicReader;
            sprintf(readername, READER_AXESSTMCLEGIC);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}