#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "generictagchip.hpp"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this       \
                                  macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C" {
LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)CHIP_GENERICTAG;
}

LIBLOGICALACCESS_API void getGenericTagChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>();
    }
}

LIBLOGICALACCESS_API void getGENERIC_T_CLChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>("GENERIC_T_CL");
    }
}

LIBLOGICALACCESS_API void
getGENERIC_T_CL_AChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>("GENERIC_T_CL_A");
    }
}

LIBLOGICALACCESS_API void
setTagIdBitsLengthOfGenericTagChip(std::shared_ptr<logicalaccess::Chip> *chip,
                                   unsigned int bits)
{
    if (chip != nullptr)
    {
        std::dynamic_pointer_cast<logicalaccess::GenericTagChip>(*chip)
            ->setTagIdBitsLength(bits);
    }
}

LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getGenericTagChip;
            sprintf(chipname, CHIP_GENERICTAG);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}