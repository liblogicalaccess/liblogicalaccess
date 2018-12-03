#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816chip.hpp>

extern "C" {
LLA_CARDS_ISO7816_API char *getLibraryName()
{
    return (char *)"ISO7816";
}

LLA_CARDS_ISO7816_API void getISO7816Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ISO7816Chip>();
    }
}

LLA_CARDS_ISO7816_API bool getChipInfoAt(unsigned int index, char *chipname,
                                         size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getISO7816Chip;
            sprintf(chipname, CHIP_ISO7816);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}