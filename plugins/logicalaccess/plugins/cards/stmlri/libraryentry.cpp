#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/stmlri/stmlri512chip.hpp>

extern "C" {
LLA_CARDS_STMLRI_API char *getLibraryName()
{
    return (char *)"StmLri512";
}

LLA_CARDS_STMLRI_API void getStmLri512Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::StmLri512Chip>();
    }
}

LLA_CARDS_STMLRI_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getStmLri512Chip;
            sprintf(chipname, CHIP_STMLRI512);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}