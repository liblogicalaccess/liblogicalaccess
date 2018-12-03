#include <string>
#include <memory>
#include <logicalaccess/plugins/cards/twic/twiccommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/twiciso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp>

extern "C" {
LLA_READERS_ISO7816_API char *getLibraryName()
{
    return (char *)"ISO7816";
}

LLA_READERS_ISO7816_API void
getTwicISO7816Commands(std::shared_ptr<logicalaccess::Commands> *commands)
{
    if (commands != nullptr)
    {
        *commands = std::make_shared<logicalaccess::TwicISO7816Commands>();
    }
}

LLA_READERS_ISO7816_API void
getDESFireISO7816Commands(std::shared_ptr<logicalaccess::Commands> *commands)
{
    if (commands != nullptr)
    {
        *commands = std::make_shared<logicalaccess::DESFireISO7816Commands>();
    }
}

LLA_READERS_ISO7816_API void
setCryptoContextDESFireISO7816Commands(std::shared_ptr<logicalaccess::Commands> *commands,
                                       std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (commands != nullptr && chip != nullptr)
    {
        std::dynamic_pointer_cast<logicalaccess::DESFireChip>(*chip)
            ->getCrypto()
            ->setCryptoContext((*chip)->getChipIdentifier());
    }
}

LLA_READERS_ISO7816_API void
getDESFireEV1ISO7816Commands(std::shared_ptr<logicalaccess::Commands> *commands)
{
    if (commands != nullptr)
    {
        *commands = std::make_shared<logicalaccess::DESFireEV1ISO7816Commands>();
    }
}

LLA_READERS_ISO7816_API void setCryptoContextDESFireEV1ISO7816Commands(
    std::shared_ptr<logicalaccess::Commands> *commands,
    std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (commands != nullptr && chip != nullptr)
    {
        std::dynamic_pointer_cast<logicalaccess::DESFireChip>(*chip)
            ->getCrypto()
            ->setCryptoContext((*chip)->getChipIdentifier());
    }
}
}