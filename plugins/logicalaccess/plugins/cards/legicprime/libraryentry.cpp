#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/legicprime/legicprimechip.hpp>

extern "C" {
LLA_CARDS_LEGICPRIME_API char *getLibraryName()
{
    return (char *)"LegicPrime";
}

LLA_CARDS_LEGICPRIME_API void
getLegicPrimeChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::LegicPrimeChip>();
    }
}

LLA_CARDS_LEGICPRIME_API bool getChipInfoAt(unsigned int index, char *chipname,
                                            size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getLegicPrimeChip;
            sprintf(chipname, CHIP_LEGICPRIME);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}