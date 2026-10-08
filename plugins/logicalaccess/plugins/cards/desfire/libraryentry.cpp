#include <cstdio>
#include <memory>

#include <logicalaccess/cards/keydiversification.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>
#include <logicalaccess/plugins/cards/duox/duoxchip.hpp>

#include <logicalaccess/plugins/cards/desfire/nxpav1keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/omnitechkeydiversification.hpp>
#include <logicalaccess/plugins/cards/desfire/sagemkeydiversification.hpp>

#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

extern "C"
{
LLA_CARDS_DESFIRE_API char *getLibraryName()
    {
        return const_cast<char *>("DESFire");
    }

    LLA_CARDS_DESFIRE_API void getDESFireChip(std::shared_ptr<logicalaccess::Chip> *chip)
    {
        if (chip == nullptr)
            return;
        *chip = std::make_shared<logicalaccess::DESFireChip>();
    }

    LLA_CARDS_DESFIRE_API void getDESFireEV1Chip(std::shared_ptr<logicalaccess::Chip> *chip)
    {
        if (chip == nullptr)
            return;
        *chip = std::make_shared<logicalaccess::DESFireEV1Chip>();
    }

    LLA_CARDS_DESFIRE_API void getDESFireEV2Chip(std::shared_ptr<logicalaccess::Chip> *chip)
    {
        if (chip == nullptr)
            return;
        *chip = std::make_shared<logicalaccess::DESFireEV2Chip>();
    }

    LLA_CARDS_DESFIRE_API void getDESFireEV3Chip(std::shared_ptr<logicalaccess::Chip> *chip)
    {
        if (chip == nullptr)
            return;
        *chip = std::make_shared<logicalaccess::DESFireEV3Chip>();
    }

    LLA_CARDS_DESFIRE_API void getDUOXChip(std::shared_ptr<logicalaccess::Chip> *chip)
    {
        if (chip == nullptr)
            return;
        *chip = std::make_shared<logicalaccess::DUOXChip>();
    }

    LLA_CARDS_DESFIRE_API void getNXPAV1Diversification(
        std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
    {
        if (keydiversification == nullptr)
            return;
        *keydiversification = std::make_shared<logicalaccess::NXPAV1KeyDiversification>();
    }

    LLA_CARDS_DESFIRE_API void getNXPAV2Diversification(
        std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
    {
        if (keydiversification == nullptr)
            return;
        *keydiversification = std::make_shared<logicalaccess::NXPAV2KeyDiversification>();
    }

    LLA_CARDS_DESFIRE_API void getSagemDiversification(
        std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
    {
        if (keydiversification == nullptr)
            return;
        *keydiversification = std::make_shared<logicalaccess::SagemKeyDiversification>();
    }

    LLA_CARDS_DESFIRE_API void getOmnitechDiversification(
        std::shared_ptr<logicalaccess::KeyDiversification> *keydiversification)
    {
        if (keydiversification == nullptr)
            return;
        *keydiversification = std::make_shared<logicalaccess::OmnitechKeyDiversification>();
    }

    namespace
    {
    using ChipFactory = void (*)(std::shared_ptr<logicalaccess::Chip> *);

    struct ChipPluginInfo
    {
        const char *name;
        ChipFactory factory;
    };

    const ChipPluginInfo CHIP_PLUGINS[] =
    {
        {CHIP_DESFIRE,     &getDESFireChip},
        {CHIP_DESFIRE_EV1, &getDESFireEV1Chip},
        {CHIP_DESFIRE_EV2, &getDESFireEV2Chip},
        {CHIP_DESFIRE_EV3, &getDESFireEV3Chip},
        {CHIP_DUOX,        &getDUOXChip}
    };

    constexpr std::size_t CHIP_PLUGIN_COUNT  = sizeof(CHIP_PLUGINS) / sizeof(CHIP_PLUGINS[0]);

    } // namespace

    LLA_CARDS_DESFIRE_API bool getChipInfoAt(unsigned int index, char *chipname, std::size_t chipnamelen, void **getterfct)
    {
        if (chipname == nullptr || getterfct == nullptr || chipnamelen != PLUGINOBJECT_MAXLEN)
            return false;
        if (index >= CHIP_PLUGIN_COUNT)
            return false;

        const ChipPluginInfo &plugin = CHIP_PLUGINS[index];
        const int written = std::snprintf(chipname, chipnamelen, "%s", plugin.name);
        if (written < 0 || static_cast<std::size_t>(written) >= chipnamelen)
            return false;

        *getterfct = reinterpret_cast<void *>(plugin.factory);
        return true;
    }

} // extern "C"