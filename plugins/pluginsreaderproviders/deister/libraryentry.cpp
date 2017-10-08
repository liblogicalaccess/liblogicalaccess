#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "deisterreaderprovider.hpp"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"Deister";
    }

    LIBLOGICALACCESS_API void getDeisterReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != nullptr)
        {
            *rp = logicalaccess::DeisterReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getDeisterReader;
                sprintf(readername, READER_DEISTER);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }
}