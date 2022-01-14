#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/yubikey5/yubikey5chip.hpp>
#include <logicalaccess/plugins/cards/yubikey5/lla_cards_yubikey5_api.hpp>

#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this       \
                                  macro, to avoid MSVC specific warnings pragma */
#endif

extern "C" {
LLA_CARDS_YUBIKEY5_API char *getLibraryName()
{
    return (char *)"Yubikey5";
}

LLA_CARDS_YUBIKEY5_API void getYubikey5Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::Yubikey5Chip>();
    }
}

LLA_CARDS_YUBIKEY5_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getYubikey5Chip;
            sprintf(chipname, CHIP_YUBIKEY5);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}