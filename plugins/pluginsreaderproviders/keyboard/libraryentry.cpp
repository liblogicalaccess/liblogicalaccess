#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "keyboardreaderprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"Keyboard";
    }

    LIBLOGICALACCESS_API void getKeyboardReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != NULL)
        {
            *rp = logicalaccess::KeyboardReaderProvider::getSingletonInstance();
        }
    }

    LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char* readername, size_t readernamelen, void** getterfct)
    {
        bool ret = false;
        if (readername != NULL && readernamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
        {
            switch (index)
            {
            case 0:
            {
                *getterfct = (void*)&getKeyboardReader;
                sprintf(readername, READER_KEYBOARD);
                ret = true;
            }
                break;
            }
        }

        return ret;
    }
}