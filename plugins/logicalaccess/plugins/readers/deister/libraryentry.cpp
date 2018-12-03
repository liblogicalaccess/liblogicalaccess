#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/deister/deisterreaderprovider.hpp>

extern "C" {
LLA_READERS_DEISTER_API char *getLibraryName()
{
    return (char *)"Deister";
}

LLA_READERS_DEISTER_API void
getDeisterReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::DeisterReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_DEISTER_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getDeisterReader;
            sprintf(readername, READER_DEISTER);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}