#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/em4135/em4135chip.hpp>
#include <logicalaccess/plugins/cards/em4135/lla_cards_em4135_api.hpp>

extern "C" {
LLA_CARDS_EM4135_API char *getLibraryName()
{
    return (char *)"EM4135";
}

LLA_CARDS_EM4135_API void getEM4135Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::EM4135Chip>();
    }
}

LLA_CARDS_EM4135_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getEM4135Chip;
            sprintf(chipname, CHIP_EM4135);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}