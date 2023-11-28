/**
 * \file desfireev1commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV1 commands.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1location.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
std::shared_ptr<DESFireEV1Chip> DESFireEV1Commands::getDESFireEV1Chip() const
{
    return std::dynamic_pointer_cast<DESFireEV1Chip>(getChip());
}

void DESFireEV1Commands::selectApplication(std::shared_ptr<DESFireEV1Location> location)
{
    selectApplication(location->aid);
    getDESFireEV1Chip()->getCrypto()->setKey(
        location->aid, 0, 0,
        getDESFireEV1Chip()->getCrypto()->getDefaultKey(location->cryptoMethod));
}

void DESFireEV1Commands::createApplication(std::shared_ptr<DESFireEV1Location> location,
                                           DESFireKeySettings settings,
                                           unsigned char maxNbKeys)
{
    createApplication(location->aid, settings, maxNbKeys, location->cryptoMethod,
                      location->useISO7816 ? FIDS_ISO_FID : FIDS_NO_ISO_FID,
                      location->useISO7816 ? location->applicationFID : 0);
}

void DESFireEV1Commands::createStdDataFile(std::shared_ptr<DESFireEV1Location> location,
                                           const DESFireAccessRights &accessRights,
                                           unsigned int fileSize)
{
    createStdDataFile(location->file, location->securityLevel, accessRights, fileSize,
                      location->useISO7816 ? location->fileFID : 0);
}
}