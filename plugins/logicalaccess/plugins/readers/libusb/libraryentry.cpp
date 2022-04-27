#include <string>
#include <memory>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderprovider.hpp>
#include <logicalaccess/plugins/readers/libusb/lla_readers_libusb_api.hpp>

extern "C" {
LLA_READERS_LIBUSB_API char *getLibraryName()
{
    return (char *)"LibUSB";
}

LLA_READERS_LIBUSB_API void getLibUSBReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::LibUSBReaderProvider::createInstance();
    }
}

LLA_READERS_LIBUSB_API bool getReaderInfoAt(unsigned int index, char *readername,
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
            *getterfct = (void *)&getLibUSBReader;
            sprintf(readername, READER_LIBUSB);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}