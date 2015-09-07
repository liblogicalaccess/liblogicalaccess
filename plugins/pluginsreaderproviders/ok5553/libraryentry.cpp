#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "ok5553readerprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"OK5553";
    }

    LIBLOGICALACCESS_API void getOK5553Reader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != NULL)
        {
            *rp = logicalaccess::OK5553ReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getOK5553Reader;
                sprintf(readername, READER_OK5553);
                ret = true;
            }
                break;
            }
        }

        return ret;
    }
}