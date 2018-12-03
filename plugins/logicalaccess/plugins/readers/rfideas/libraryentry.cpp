#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/rfideas/rfideasreaderprovider.hpp>

extern "C" {
LLA_READERS_RFIDEAS_API char *getLibraryName()
{
    return (char *)"RFIDeas";
}

LLA_READERS_RFIDEAS_API void
getRFIDeasReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::RFIDeasReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_RFIDEAS_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getRFIDeasReader;
            sprintf(readername, READER_RFIDEAS);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}