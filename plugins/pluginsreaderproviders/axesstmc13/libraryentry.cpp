#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "axesstmc13readerprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"AxessTMC13";
    }

    LIBLOGICALACCESS_API void getAxessTMC13Reader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != nullptr)
        {
            *rp = logicalaccess::AxessTMC13ReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getAxessTMC13Reader;
                sprintf(readername, READER_AXESSTMC13);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }
}