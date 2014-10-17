#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "indalachip.hpp"

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
        return (char *)"Indala";
    }

    LIBLOGICALACCESS_API void getIndalaChip(std::shared_ptr<logicalaccess::Chip>* chip)
    {
        if (chip != NULL)
        {
            *chip = std::shared_ptr<logicalaccess::IndalaChip>(new logicalaccess::IndalaChip());
        }
    }

    LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char* chipname, size_t chipnamelen, void** getterfct)
    {
        bool ret = false;
        if (chipname != NULL && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
        {
            switch (index)
            {
            case 0:
            {
                *getterfct = (void*)&getIndalaChip;
                sprintf(chipname, CHIP_INDALA);
                ret = true;
            }
                break;
            }
        }

        return ret;
    }
}