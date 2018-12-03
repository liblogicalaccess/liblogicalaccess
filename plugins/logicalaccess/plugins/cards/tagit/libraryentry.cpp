#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/tagit/tagitchip.hpp>

extern "C" {
LLA_CARDS_TAGIT_API char *getLibraryName()
{
    return (char *)"TagIt";
}

LLA_CARDS_TAGIT_API void getTagItChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::TagItChip>();
    }
}

LLA_CARDS_TAGIT_API bool getChipInfoAt(unsigned int index, char *chipname,
                                       size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getTagItChip;
            sprintf(chipname, CHIP_TAGIT);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}