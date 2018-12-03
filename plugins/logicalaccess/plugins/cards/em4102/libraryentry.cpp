#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/em4102/em4102chip.hpp>
#include <logicalaccess/plugins/cards/em4102/lla_cards_em4102_api.hpp>

extern "C" {
LLA_CARDS_EM4102_API char *getLibraryName()
{
    return (char *)"EM4102";
}

LLA_CARDS_EM4102_API void getEM4102Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::EM4102Chip>();
    }
}

LLA_CARDS_EM4102_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getEM4102Chip;
            sprintf(chipname, CHIP_EM4102);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}