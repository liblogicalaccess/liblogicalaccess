#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/cps3/cps3chip.hpp>
#include <logicalaccess/plugins/cards/cps3/lla_cards_cps3_api.hpp>

#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this       \
                                  macro, to avoid MSVC specific warnings pragma */
#endif

extern "C" {
LLA_CARDS_CPS3_API char *getLibraryName()
{
    return (char *)"CPS3";
}

LLA_CARDS_CPS3_API void getCPS3Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::CPS3Chip>();
    }
}

LLA_CARDS_CPS3_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getCPS3Chip;
            sprintf(chipname, CHIP_CPS3);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}