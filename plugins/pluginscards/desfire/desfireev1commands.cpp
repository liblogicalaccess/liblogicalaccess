/**
 * \file desfireev1commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 commands.
 */

#include "desfireev1commands.hpp"
#include "desfireev1chip.hpp"

namespace logicalaccess
{
    std::shared_ptr<DESFireEV1Chip> DESFireEV1Commands::getDESFireEV1Chip() const
    {
        return std::dynamic_pointer_cast<DESFireEV1Chip>(getChip());
    }

    void DESFireEV1Commands::selectApplication(std::shared_ptr<DESFireLocation> location)
    {
        std::shared_ptr<DESFireEV1Location> evlocation = std::dynamic_pointer_cast<DESFireEV1Location>(location);
        DESFireCommands::selectApplication(location);
        if (evlocation)
        {
            getDESFireEV1Chip()->getDESFireProfile()->setKey(evlocation->aid, 0, getDESFireEV1Chip()->getDESFireProfile()->getDefaultKey(evlocation->cryptoMethod));
        }
    }

    void DESFireEV1Commands::createApplication(std::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, unsigned char maxNbKeys)
    {
        std::shared_ptr<DESFireEV1Location> evlocation = std::dynamic_pointer_cast<DESFireEV1Location>(location);
        if (evlocation)
        {
            createApplication(location->aid, settings, maxNbKeys, evlocation->cryptoMethod, evlocation->useISO7816 ? FIDS_ISO_FID : FIDS_NO_ISO_FID, evlocation->useISO7816 ? evlocation->applicationFID : 0);
        }
        else
        {
            DESFireCommands::createApplication(location, settings, maxNbKeys);
        }
    }

    void DESFireEV1Commands::createStdDataFile(std::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        std::shared_ptr<DESFireEV1Location> evlocation = std::dynamic_pointer_cast<DESFireEV1Location>(location);
        if (evlocation)
        {
            createStdDataFile(location->file, location->securityLevel, accessRights, fileSize, evlocation->useISO7816 ? evlocation->fileFID : 0);
        }
        else
        {
            DESFireCommands::createStdDataFile(location, accessRights, fileSize);
        }
    }
}