#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/icode2/icode2chip.hpp>
#include <logicalaccess/plugins/cards/icode2/lla_cards_icode2_api.hpp>

extern "C" {
LLA_CARDS_ICODE2_API char *getLibraryName()
{
    return (char *)"iCode2";
}

LLA_CARDS_ICODE2_API void getiCode2Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ICode2Chip>();
    }
}

LLA_CARDS_ICODE2_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getiCode2Chip;
            sprintf(chipname, CHIP_ICODE2);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}