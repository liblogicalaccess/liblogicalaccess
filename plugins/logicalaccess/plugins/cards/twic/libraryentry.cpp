#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/twic/twicchip.hpp>

extern "C" {
LLA_CARDS_TWIC_API char *getLibraryName()
{
    return (char *)"Twic";
}

LLA_CARDS_TWIC_API void getTwicChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::TwicChip>();
    }
}

LLA_CARDS_TWIC_API bool getChipInfoAt(unsigned int index, char *chipname,
                                      size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getTwicChip;
            sprintf(chipname, CHIP_TWIC);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}