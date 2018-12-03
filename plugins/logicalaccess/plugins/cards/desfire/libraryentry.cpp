#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/sagemkeydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav1keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/omnitechkeydiversification.hpp>
#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

extern "C" {
LLA_CARDS_DESFIRE_API char *getLibraryName()
{
    return (char *)"DESFire";
}

LLA_CARDS_DESFIRE_API void getDESFireEV1Chip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::DESFireEV1Chip>();
    }
}

LLA_CARDS_DESFIRE_API void getDESFireChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::DESFireChip>();
    }
}

LLA_CARDS_DESFIRE_API void getNXPAV1Diversification(
    std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
{
    if (keydiversification != nullptr)
    {
        *keydiversification = std::make_shared<logicalaccess::NXPAV1KeyDiversification>();
    }
}

LLA_CARDS_DESFIRE_API void getNXPAV2Diversification(
    std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
{
    if (keydiversification != nullptr)
    {
        *keydiversification = std::make_shared<logicalaccess::NXPAV2KeyDiversification>();
    }
}

LLA_CARDS_DESFIRE_API void getSagemDiversification(
    std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
{
    if (keydiversification != nullptr)
    {
        *keydiversification = std::make_shared<logicalaccess::SagemKeyDiversification>();
    }
}

LLA_CARDS_DESFIRE_API void getOmnitechDiversification(
    std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
{
    if (keydiversification != nullptr)
    {
        *keydiversification =
            std::make_shared<logicalaccess::OmnitechKeyDiversification>();
    }
}

LLA_CARDS_DESFIRE_API bool getChipInfoAt(unsigned int index, char *chipname,
                                         size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getDESFireChip;
            sprintf(chipname, CHIP_DESFIRE);
            ret = true;
        }
        break;

        case 1:
        {
            *getterfct = (void *)&getDESFireEV1Chip;
            sprintf(chipname, CHIP_DESFIRE_EV1);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}