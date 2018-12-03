#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>

extern "C" {
LLA_CARDS_ISO15693_API char *getLibraryName()
{
    return (char *)"ISO15693";
}

LLA_CARDS_ISO15693_API void getISO15693Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ISO15693Chip>();
    }
}

LLA_CARDS_ISO15693_API bool getChipInfoAt(unsigned int index, char *chipname,
                                          size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getISO15693Chip;
            sprintf(chipname, CHIP_ISO15693);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}