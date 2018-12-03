#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/samav2/samav1chip.hpp>
#include <logicalaccess/plugins/cards/samav2/samav2chip.hpp>

extern "C" {
LLA_CARDS_SAMAV2_API char *getLibraryName()
{
    return (char *)"SAM";
}

LLA_CARDS_SAMAV2_API void getSAM_AV1Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::SAMAV1Chip>();
    }
}

LLA_CARDS_SAMAV2_API void getSAM_AV2Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::SAMAV2Chip>();
    }
}

LLA_CARDS_SAMAV2_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getSAM_AV1Chip;
            sprintf(chipname, CHIP_SAMAV1);
            ret = true;
        }
        break;
        case 1:
        {
            *getterfct = (void *)&getSAM_AV2Chip;
            sprintf(chipname, CHIP_SAMAV2);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}