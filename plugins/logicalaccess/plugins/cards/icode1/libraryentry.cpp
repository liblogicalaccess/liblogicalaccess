#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/icode1/icode1chip.hpp>
#include <logicalaccess/plugins/cards/icode1/lla_cards_icode1_api.hpp>

extern "C" {
LLA_CARDS_ICODE1_API char *getLibraryName()
{
    return (char *)"iCode1";
}

LLA_CARDS_ICODE1_API void getiCode1Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ICode1Chip>();
    }
}

LLA_CARDS_ICODE1_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getiCode1Chip;
            sprintf(chipname, CHIP_ICODE1);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}