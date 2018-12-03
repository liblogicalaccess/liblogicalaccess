#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/prox/proxchip.hpp>

extern "C" {
LLA_CARDS_PROX_API char *getLibraryName()
{
    return (char *)"Prox";
}

LLA_CARDS_PROX_API void getProxChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ProxChip>();
    }
}

LLA_CARDS_PROX_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getProxChip;
            sprintf(chipname, CHIP_PROX);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}