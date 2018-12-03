#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderprovider.hpp>


extern "C" {
LLA_READERS_RPLETH_API char *getLibraryName()
{
    return (char *)"Rpleth";
}

LLA_READERS_RPLETH_API void
getRplethReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::RplethReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_RPLETH_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getRplethReader;
            sprintf(readername, READER_RPLETH);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}