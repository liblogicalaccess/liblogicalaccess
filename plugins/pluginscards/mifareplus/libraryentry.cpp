#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "MifarePlusSL1Chip.hpp"
#include "MifarePlusSL0Chip.hpp"
#include "MifarePlusSL3Chip.hpp"

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
    return (char *) "MifarePlus";
}

LIBLOGICALACCESS_API void getMifarePlus_SL1_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusSL1_4kChip>(
                new logicalaccess::MifarePlusSL1_4kChip());
    }
}


LIBLOGICALACCESS_API void getMifarePlus_SL1_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusSL1_2kChip>(
                new logicalaccess::MifarePlusSL1_2kChip());
    }
}


LIBLOGICALACCESS_API void getMifarePlus_SL0_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusSL0_4kChip>(
                new logicalaccess::MifarePlusSL0_4kChip());
    }
}


LIBLOGICALACCESS_API void getMifarePlus_SL0_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusSL0_2kChip>(
                new logicalaccess::MifarePlusSL0_2kChip());
    }
}

LIBLOGICALACCESS_API void getMifarePlus_SL3_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::Chip>(
                new logicalaccess::MifarePlusSL3Chip(false));
    }
}

LIBLOGICALACCESS_API void getMifarePlus_SL3_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::Chip>(
                new logicalaccess::MifarePlusSL3Chip(true));
    }
}

/**
 * Unknown security level.
 */
LIBLOGICALACCESS_API void getMifarePlusSChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusChip>(
                new logicalaccess::MifarePlusChip("MifarePlusS"));
    }
}

/**
 * Unknown security level.
 */
LIBLOGICALACCESS_API void getMifarePlusXChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::shared_ptr<logicalaccess::MifarePlusChip>(
                new logicalaccess::MifarePlusChip("MifarePlusX"));
    }
}

LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char *chipname, size_t chipnamelen,
                                        void **getterfct)
{
    bool ret = false;
    if (chipname != NULL && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
    {
        switch (index)
        {
            case 0:
            {
                *getterfct = (void *) &getMifarePlus_SL0_4KChip;
                sprintf(chipname, "MifarePlus_SL0_4K");
                ret = true;
            }

            case 1:
            {
                *getterfct = (void *) &getMifarePlus_SL0_2KChip;
                sprintf(chipname, "MifarePlus_SL0_2K");
                ret = true;
            }

            case 2:
            {
                *getterfct = (void *) &getMifarePlus_SL1_4KChip;
                sprintf(chipname, "MifarePlus_SL1_4K");
                ret = true;
            }

            case 3:
            {
                *getterfct = (void *) &getMifarePlus_SL1_2KChip;
                sprintf(chipname, "MifarePlus_SL1_2K");
                ret = true;
            }

            case 4:
            {
                *getterfct = (void *) &getMifarePlus_SL1_4KChip;
                sprintf(chipname, "MifarePlus_SL3_4K");
                ret = true;
            }

            case 5:
            {
                *getterfct = (void *) &getMifarePlus_SL3_2KChip;
                sprintf(chipname, "MifarePlus_SL3_2K");
                ret = true;
            }

            case 6:
            {
                *getterfct = (void *) &getMifarePlusSChip;
                sprintf(chipname, "MifarePlusS");
                ret = true;
            }
            case 7:
            {
                *getterfct = (void *) &getMifarePlusXChip;
                sprintf(chipname, "MifarePlusX");
                ret = true;
            }
                break;
        }
    }

    return ret;
}
}