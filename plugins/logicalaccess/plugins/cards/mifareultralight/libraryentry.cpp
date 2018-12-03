#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcchip.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightchip.hpp>

extern "C" {
LLA_CARDS_MIFAREULTRALIGHT_API char *getLibraryName()
{
    return (char *)"MifareUltralight";
}

LLA_CARDS_MIFAREULTRALIGHT_API void
getMifareUltralightCChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifareUltralightCChip>();
    }
}

LLA_CARDS_MIFAREULTRALIGHT_API void
getMifareUltralightChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifareUltralightChip>();
    }
}

LLA_CARDS_MIFAREULTRALIGHT_API bool getChipInfoAt(unsigned int index, char *chipname,
                                                  size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getMifareUltralightChip;
            sprintf(chipname, CHIP_MIFAREULTRALIGHT);
            ret = true;
        }
        break;
        case 1:
        {
            *getterfct = (void *)&getMifareUltralightCChip;
            sprintf(chipname, CHIP_MIFAREULTRALIGHTC);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}