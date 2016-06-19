#include "epasschip.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/readerproviders/readerunit.hpp>

extern "C" {
LIBLOGICALACCESS_API char *getLibraryName()
{
    return (char *)"Epass";
}

LIBLOGICALACCESS_API void getEPassChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != NULL)
    {
        *chip = std::make_shared<logicalaccess::EPassChip>();
    }
}

LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != NULL && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
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
        }
    }
    return ret;
}
}
