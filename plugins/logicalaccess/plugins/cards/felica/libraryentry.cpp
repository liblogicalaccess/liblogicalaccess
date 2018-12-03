#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/felica/felicachip.hpp>
#include <logicalaccess/plugins/cards/felica/lla_cards_felica_api.hpp>

extern "C" {
LLA_CARDS_FELICA_API char *getLibraryName()
{
    return (char *)"FeliCa";
}

LLA_CARDS_FELICA_API void getFeliCaChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::FeliCaChip>();
    }
}

LLA_CARDS_FELICA_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getFeliCaChip;
            sprintf(chipname, CHIP_FELICA);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}