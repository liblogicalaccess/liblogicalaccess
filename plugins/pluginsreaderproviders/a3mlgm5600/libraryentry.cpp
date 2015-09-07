#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "a3mlgm5600readerprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"A3MLGM5600";
    }

    LIBLOGICALACCESS_API void getA3MLGM5600Reader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != NULL)
        {
            *rp = logicalaccess::A3MLGM5600ReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getA3MLGM5600Reader;
                sprintf(readername, READER_A3MLGM5600);
                ret = true;
            }
                break;
            }
        }

        return ret;
    }
}