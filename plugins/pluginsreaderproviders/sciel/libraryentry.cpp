#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "scielreaderprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"SCIEL";
    }

    LIBLOGICALACCESS_API void getSCIELReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != nullptr)
        {
            *rp = logicalaccess::SCIELReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getSCIELReader;
                sprintf(readername, READER_SCIEL);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }
}