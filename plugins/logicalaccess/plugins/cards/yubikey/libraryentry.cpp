#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeychip.hpp>
#include <logicalaccess/plugins/cards/yubikey/lla_cards_yubikey_api.hpp>

#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this       \
                                  macro, to avoid MSVC specific warnings pragma */
#endif

extern "C" {
LLA_CARDS_YUBIKEY_API char *getLibraryName()
{
    return (char *)"Yubikey";
}

LLA_CARDS_YUBIKEY_API void getYubikeyChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::YubikeyChip>();
    }
}

LLA_CARDS_YUBIKEY_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getYubikeyChip;
            sprintf(chipname, CHIP_YUBIKEY);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}