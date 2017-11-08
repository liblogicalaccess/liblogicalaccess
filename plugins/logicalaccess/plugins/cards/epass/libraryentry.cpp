#include <logicalaccess/plugins/cards/epass/epasschip.hpp>
#include <logicalaccess/logicalaccess_api.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/readerproviders/readerunit.hpp>

extern "C" {
LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"Epass";
}

LIBLOGICALACCESS_API void getEPassChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::EPassChip>();
    }
}

LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getEPassChip;
            sprintf(chipname, "EPass");
            ret = true;
        }
        break;
        default:;
        }
    }
    return ret;
}
}
