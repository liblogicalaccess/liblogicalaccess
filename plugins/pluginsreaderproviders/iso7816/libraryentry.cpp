#include <string>
#include <memory>
#include "twiccommands.hpp"
#include "commands/twiciso7816commands.hpp"
#include "commands/desfireiso7816commands.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "logicalaccess/logicalaccess_api.hpp"

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"ISO7816";
    }

    LIBLOGICALACCESS_API void getTwicISO7816Commands(std::shared_ptr<logicalaccess::Commands>* commands)
    {
        if (commands != NULL)
        {
            *commands = std::shared_ptr<logicalaccess::TwicISO7816Commands>(new logicalaccess::TwicISO7816Commands());
        }
    }

    LIBLOGICALACCESS_API void getDESFireISO7816Commands(std::shared_ptr<logicalaccess::Commands>* commands)
    {
        if (commands != NULL)
        {
            *commands = std::shared_ptr<logicalaccess::DESFireISO7816Commands>(new logicalaccess::DESFireISO7816Commands());
        }
    }

    LIBLOGICALACCESS_API void setCryptoContextDESFireISO7816Commands(std::shared_ptr<logicalaccess::Commands>* commands, std::shared_ptr<logicalaccess::Chip>* chip)
    {
        if (commands != NULL && chip != NULL)
        {
            std::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(*commands)->getCrypto()->setCryptoContext(std::dynamic_pointer_cast<logicalaccess::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
        }
    }

    LIBLOGICALACCESS_API void getDESFireEV1ISO7816Commands(std::shared_ptr<logicalaccess::Commands>* commands)
    {
        if (commands != NULL)
        {
            *commands = std::shared_ptr<logicalaccess::DESFireEV1ISO7816Commands>(new logicalaccess::DESFireEV1ISO7816Commands());
        }
    }

    LIBLOGICALACCESS_API void setCryptoContextDESFireEV1ISO7816Commands(std::shared_ptr<logicalaccess::Commands>* commands, std::shared_ptr<logicalaccess::Chip>* chip)
    {
        if (commands != NULL && chip != NULL)
        {
            std::dynamic_pointer_cast<logicalaccess::DESFireEV1ISO7816Commands>(*commands)->getCrypto()->setCryptoContext(std::dynamic_pointer_cast<logicalaccess::DESFireProfile>((*chip)->getProfile()), (*chip)->getChipIdentifier());
        }
    }
}