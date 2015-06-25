#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "admittoreaderprovider.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"Admitto";
    }

    LIBLOGICALACCESS_API void getAdmittoReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != NULL)
        {
            *rp = logicalaccess::AdmittoReaderProvider::getSingletonInstance();
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
                *getterfct = (void*)&getAdmittoReader;
                sprintf(readername, READER_ADMITTO);
                ret = true;
            }
                break;
            }
        }

        return ret;
    }
}