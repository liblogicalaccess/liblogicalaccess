#include <string>
#include <logicalaccess/plugins/readers/osdp/osdpreaderprovider.hpp>

extern "C" {
LLA_READERS_OSDP_API char *getLibraryName()
{
    return (char *)"OSDP";
}

LLA_READERS_OSDP_API void
getOSDPReader(std::shared_ptr<logicalaccess::OSDPReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::OSDPReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_OSDP_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getOSDPReader;
            sprintf(readername, READER_OSDP);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}
