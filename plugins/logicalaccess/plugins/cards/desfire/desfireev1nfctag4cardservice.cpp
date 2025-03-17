/**
 * \file desfireev1nfctag4cardservice.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief DESFireEV1 NFC Tag type 4 CardService.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev1nfctag4cardservice.hpp>

namespace logicalaccess
{
DESFireEV1NFCTag4CardService::DESFireEV1NFCTag4CardService(std::shared_ptr<Chip> chip) : ISO7816NFCTag4CardService(chip)
{
    d_app_empty_key = std::make_shared<DESFireKey>("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    d_app_empty_key->setKeyType(DF_KEY_AES);
}

void DESFireEV1NFCTag4CardService::createNFCApplication(unsigned int aid,
    unsigned short isoFIDApplication, unsigned short isoFIDCapabilityContainer,
    unsigned short isoFIDNDEFFile, unsigned short NDEFFileSize)
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));
    std::shared_ptr<DESFireEV1Commands> desfireev1command(
        std::dynamic_pointer_cast<DESFireEV1Commands>(getChip()->getCommands()));

    desfirecommand->selectApplication(0);
    if (d_picc_key)
    {
        try
        {
            desfirecommand->authenticate(0, d_picc_key);
        }
        catch (std::exception &)
        {
        }
    }

    unsigned char dfName[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};
    ByteVector dfNameVector(dfName, dfName + 7);
    desfireev1command->createApplication(aid, KS_DEFAULT, 3, d_app_empty_key->getKeyType(), FIDS_ISO_FID,
                                         isoFIDApplication, dfNameVector);

    desfirecommand->selectApplication(aid);
    desfirecommand->authenticate(0x00, d_app_empty_key);

    DESFireAccessRights dar;
    dar.changeAccess       = AR_KEY0;
    dar.readAccess         = AR_FREE;
    dar.readAndWriteAccess = AR_KEY0;
    dar.writeAccess        = AR_FREE; // AR_KEY0
    desfireev1command->createStdDataFile(0x01, CM_PLAIN, dar, 0x0f,
                                         isoFIDCapabilityContainer);
                                    
    dar.changeAccess       = AR_KEY0;
    dar.readAccess         = AR_FREE;
    dar.readAndWriteAccess = AR_FREE;
    dar.writeAccess        = AR_FREE;
    createNDEFFile(dar, isoFIDNDEFFile);

    if (d_app_new_key)
    {
        desfirecommand->changeKey(2, d_app_new_key);
        desfirecommand->changeKey(1, d_app_new_key);
        desfirecommand->changeKey(0, d_app_new_key);
    }

    auto iso7816command = getISO7816Commands();
    iso7816command->selectFile(dfNameVector);
    writeCapabilityContainer(isoFIDCapabilityContainer, isoFIDNDEFFile, NDEFFileSize);
}

void DESFireEV1NFCTag4CardService::deleteNFCApplication(unsigned int aid) const
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));

    if (d_picc_key)
    {
        desfirecommand->selectApplication(0);
        desfirecommand->authenticate(0, d_picc_key);
    }
    else
    {
        desfirecommand->selectApplication(aid);
        desfirecommand->authenticate(0, d_app_new_key ? d_app_new_key : d_app_empty_key);
    }
    desfirecommand->deleteApplication(aid);
}

void DESFireEV1NFCTag4CardService::createNDEFFile(const DESFireAccessRights& dar, unsigned short isoFIDNDEFFile)
{
    auto df1cmd = std::dynamic_pointer_cast<DESFireEV1Commands>(getChip()->getCommands());
    df1cmd->createStdDataFile(0x02, CM_PLAIN, dar, 0xff, isoFIDNDEFFile);
}

std::shared_ptr<NdefMessage>
DESFireEV1NFCTag4CardService::readNDEFFile(const ByteVector &appDFName,
                                           unsigned short isoFIDNDEFFile)
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));

    desfirecommand->selectApplication(0);
    return ISO7816NFCTag4CardService::readNDEFFile(appDFName, isoFIDNDEFFile);
}

void DESFireEV1NFCTag4CardService::writeNDEF(std::shared_ptr<NdefMessage> records)
{
    createNFCApplication(0x000001);
    writeNDEFFile(records);
}

void DESFireEV1NFCTag4CardService::eraseNDEF()
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));
    desfirecommand->selectApplication(0);
    desfirecommand->authenticate(0, d_picc_key);
    desfirecommand->erase();
}

std::shared_ptr<ISO7816Commands> DESFireEV1NFCTag4CardService::getISO7816Commands() const
{
    return getDESFireChip()->getDESFireEV1Commands()->getISO7816Commands();
}
}