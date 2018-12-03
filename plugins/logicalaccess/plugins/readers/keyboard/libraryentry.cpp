#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/keyboard/keyboardreaderprovider.hpp>
#include <logicalaccess/plugins/readers/keyboard/lla_readers_private_keyboard_api.hpp>


extern "C" {
LLA_READERS_PRIVATE_KEYBOARD_API char *getLibraryName()
{
    return (char *)"Keyboard";
}

LLA_READERS_PRIVATE_KEYBOARD_API void
getKeyboardReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::KeyboardReaderProvider::getSingletonInstance();
    }
}

LLA_READERS_PRIVATE_KEYBOARD_API bool getReaderInfoAt(unsigned int index,
                                                      char *readername,
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
            *getterfct = (void *)&getKeyboardReader;
            sprintf(readername, READER_KEYBOARD);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}