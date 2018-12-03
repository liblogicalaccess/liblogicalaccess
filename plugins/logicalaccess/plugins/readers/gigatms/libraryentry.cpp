#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/gigatms/gigatmsreaderprovider.hpp>

extern "C" {
LLA_READERS_GIGATMS_API char *getLibraryName()
{
    return (char *)"GigaTMS";
}

LLA_READERS_GIGATMS_API void
getGigaTMSReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::GigaTMSReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_GIGATMS_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getGigaTMSReader;
            sprintf(readername, READER_GIGATMS);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}