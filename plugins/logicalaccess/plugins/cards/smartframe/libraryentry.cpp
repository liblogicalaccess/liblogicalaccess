#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/smartframe/smartframechip.hpp>

extern "C" {
LLA_CARDS_SMARTFRAME_API char *getLibraryName()
{
    return (char *)"SmartFrame";
}

LLA_CARDS_SMARTFRAME_API void
getSmartFrameChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::SmartFrameChip>();
    }
}

LLA_CARDS_SMARTFRAME_API bool getChipInfoAt(unsigned int index, char *chipname,
                                            size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getSmartFrameChip;
            sprintf(chipname, CHIP_SMARTFRAME);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}