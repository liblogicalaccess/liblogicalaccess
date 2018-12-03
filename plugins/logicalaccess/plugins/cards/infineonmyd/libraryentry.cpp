#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/infineonmyd/infineonmydchip.hpp>

extern "C" {
LLA_CARDS_INFINEONMYD_API char *getLibraryName()
{
    return (char *)"InfineonMYD";
}

LLA_CARDS_INFINEONMYD_API void
getInfineonMYDChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::InfineonMYDChip>();
    }
}

LLA_CARDS_INFINEONMYD_API bool getChipInfoAt(unsigned int index, char *chipname,
                                             size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getInfineonMYDChip;
            sprintf(chipname, CHIP_INFINEONMYD);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}