#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/gunnebo/gunneboreaderprovider.hpp>

extern "C" {
LLA_READERS_GUNNEBO_API char *getLibraryName()
{
    return (char *)"Gunnebo";
}

LLA_READERS_GUNNEBO_API void
getGunneboReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::GunneboReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_GUNNEBO_API bool getReaderInfoAt(unsigned int index, char *readername,
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