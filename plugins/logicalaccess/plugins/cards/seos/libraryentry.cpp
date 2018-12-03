#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/seos/seoschip.hpp>

extern "C" {
LLA_CARDS_SEOS_API char *getLibraryName()
{
    return (char *)"SEOS";
}

LLA_CARDS_SEOS_API void getSEOSChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::SEOSChip>();
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
            *getterfct = (void *)&getSEOSChip;
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