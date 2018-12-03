#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/admitto/admittoreaderprovider.hpp>

extern "C" {
LLA_READERS_ADMITTO_API char *getLibraryName()
{
    return (char *)"Admitto";
}

LLA_READERS_ADMITTO_API void
getAdmittoReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::AdmittoReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_ADMITTO_API bool getReaderInfoAt(const unsigned int index, char *readername,
                                             const size_t readernamelen, void **getterfct)
{
    bool ret = false;
    if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN &&
        getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getAdmittoReader;
            sprintf(readername, READER_ADMITTO);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}