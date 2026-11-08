#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/nedapmobileaccess/nedapmobileaccesschip.hpp>
#include <logicalaccess/plugins/cards/nedapmobileaccess/lla_cards_nedapmobileaccess_api.hpp>

extern "C" {
LLA_CARDS_NEDAPMOBILEACCESS_API char *getLibraryName()
{
    return (char *)"NedapMobileAccess";
}

LLA_CARDS_NEDAPMOBILEACCESS_API void
getNedapMobileAccessChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::NedapMobileAccessChip>();
    }
}

LLA_CARDS_NEDAPMOBILEACCESS_API bool getChipInfoAt(unsigned int index, char *chipname,
                                                   size_t chipnamelen, void **getterfct)
{
    if (chipname == nullptr || chipnamelen != PLUGINOBJECT_MAXLEN || getterfct == nullptr ||
        index != 0)
    {
        return false;
    }

    *getterfct = (void *)&getNedapMobileAccessChip;
    sprintf(chipname, CHIP_NEDAP_MOBILE_ACCESS);
    return true;
}
}
