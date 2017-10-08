#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "smartidreaderprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"SmartID";
    }

    LIBLOGICALACCESS_API void getSmartIDReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != nullptr)
        {
            *rp = logicalaccess::SmartIDReaderProvider::getSingletonInstance();
        }
    }

    LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char* readername, size_t readernamelen, void** getterfct)
    {
        bool ret = false;
        if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
        {
            switch (index)
            {
            case 0:
            {
                *getterfct = (void*)&getSmartIDReader;
                sprintf(readername, READER_SMARTID);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }
}