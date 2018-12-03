#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/proxlite/proxlitechip.hpp>

extern "C" {
LLA_CARDS_PROXLITE_API char *getLibraryName()
{
    return (char *)"ProxLite";
}

LLA_CARDS_PROXLITE_API void getProxLiteChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::ProxLiteChip>();
    }
}

LLA_CARDS_PROXLITE_API bool getChipInfoAt(unsigned int index, char *chipname,
                                          size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getProxLiteChip;
            sprintf(chipname, CHIP_PROXLITE);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}