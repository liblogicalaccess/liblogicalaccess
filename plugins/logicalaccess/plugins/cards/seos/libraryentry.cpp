#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/seos/seoschip.hpp>

extern "C" {
LLA_CARDS_SEOS_API char *getLibraryName()
{
    return (char *)"SEOS";
}

LLA_CARDS_SEOS_API void getSeosChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::SeosChip>();
    }
}

LLA_CARDS_SEOS_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getSeosChip;
            sprintf(chipname, CHIP_SEOS);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}