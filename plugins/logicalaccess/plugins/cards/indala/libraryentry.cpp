#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/indala/indalachip.hpp>
#include <logicalaccess/plugins/cards/indala/lla_cards_indala_api.hpp>

extern "C" {
LLA_CARDS_INDALA_API char *getLibraryName()
{
    return (char *)"Indala";
}

LLA_CARDS_INDALA_API void getIndalaChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::IndalaChip>();
    }
}

LLA_CARDS_INDALA_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getIndalaChip;
            sprintf(chipname, CHIP_INDALA);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}