#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/a3mlgm5600/a3mlgm5600readerprovider.hpp>

extern "C" {
LLA_READERS_A3MLGM5600_API char *getLibraryName()
{
    return (char *)"A3MLGM5600";
}

LLA_READERS_A3MLGM5600_API void
getA3MLGM5600Reader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::A3MLGM5600ReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_A3MLGM5600_API bool getReaderInfoAt(const unsigned int index,
                                                char *readername,
                                                const size_t readernamelen,
                                                void **getterfct)
{
    bool ret = false;
    if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN &&
        getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getA3MLGM5600Reader;
            sprintf(readername, READER_A3MLGM5600);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}