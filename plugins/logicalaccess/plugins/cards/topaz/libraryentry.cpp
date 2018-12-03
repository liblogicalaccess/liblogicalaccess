#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/topaz/topazchip.hpp>

extern "C" {
LLA_CARDS_TOPAZ_API char *getLibraryName()
{
    return (char *)"Topaz";
}

LLA_CARDS_TOPAZ_API void getTopazChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::TopazChip>();
    }
}

LLA_CARDS_TOPAZ_API bool getChipInfoAt(unsigned int index, char *chipname,
                                       size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getTopazChip;
            sprintf(chipname, CHIP_TOPAZ);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}