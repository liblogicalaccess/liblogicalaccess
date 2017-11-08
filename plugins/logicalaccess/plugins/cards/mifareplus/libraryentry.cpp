#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/mifareplus/MifarePlusSL1Chip.hpp>
#include <logicalaccess/plugins/cards/mifareplus/MifarePlusSL0Chip.hpp>
#include <logicalaccess/plugins/cards/mifareplus/MifarePlusSL3Chip.hpp>
#include <logicalaccess/plugins/cards/mifareplus/mifareplusschip.hpp>
#include <logicalaccess/plugins/cards/mifareplus/mifareplusxchip.hpp>

#ifdef _MSC_VER
#include <logicalaccess/msliblogicalaccess.h>
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
    return (char *)"MifarePlus";
}

LIBLOGICALACCESS_API void
getMifarePlus_SL1_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusSL1_4kChip>();
    }
}


LIBLOGICALACCESS_API void
getMifarePlus_SL1_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusSL1_2kChip>();
    }
}


LIBLOGICALACCESS_API void
getMifarePlus_SL0_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusSL0_4kChip>();
    }
}


LIBLOGICALACCESS_API void
getMifarePlus_SL0_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusSL0_2kChip>();
    }
}

LIBLOGICALACCESS_API void
getMifarePlus_SL3_4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::static_pointer_cast<logicalaccess::Chip>(
            std::make_shared<logicalaccess::MifarePlusSL3Chip>(false));
    }
}

LIBLOGICALACCESS_API void
getMifarePlus_SL3_2KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::static_pointer_cast<logicalaccess::Chip>(
            std::make_shared<logicalaccess::MifarePlusSL3Chip>(true));
    }
}

/**
 * Unknown security level.
 */
LIBLOGICALACCESS_API void getMifarePlusSChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusSChip>();
    }
}

/**
 * Unknown security level.
 */
LIBLOGICALACCESS_API void getMifarePlusXChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifarePlusXChip>();
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
            *getterfct = (void *)&getMifarePlus_SL0_4KChip;
            sprintf(chipname, "MifarePlus_SL0_4K");
            ret = true;
            break;
        }

        case 1:
        {
            *getterfct = (void *)&getMifarePlus_SL0_2KChip;
            sprintf(chipname, "MifarePlus_SL0_2K");
            ret = true;
            break;
        }

        case 2:
        {
            *getterfct = (void *)&getMifarePlus_SL1_4KChip;
            sprintf(chipname, "MifarePlus_SL1_4K");
            ret = true;
            break;
        }

        case 3:
        {
            *getterfct = (void *)&getMifarePlus_SL1_2KChip;
            sprintf(chipname, "MifarePlus_SL1_2K");
            ret = true;
            break;
        }

        case 4:
        {
            *getterfct = (void *)&getMifarePlus_SL1_4KChip;
            sprintf(chipname, "MifarePlus_SL3_4K");
            ret = true;
            break;
        }

        case 5:
        {
            *getterfct = (void *)&getMifarePlus_SL3_2KChip;
            sprintf(chipname, "MifarePlus_SL3_2K");
            ret = true;
            break;
        }

        case 6:
        {
            *getterfct = (void *)&getMifarePlusSChip;
            sprintf(chipname, "MifarePlusS");
            ret = true;
            break;
        }
        case 7:
        {
            *getterfct = (void *)&getMifarePlusXChip;
            sprintf(chipname, "MifarePlusX");
            ret = true;
            break;
        }
        default:;
        }
    }

    return ret;
}
}