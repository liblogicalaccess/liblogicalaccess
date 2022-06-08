/**
 * \file desfireev1nfctag4cardservice.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief DESFireEV1 NFC Tag type 4 CardService.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev1nfctag4cardservice.hpp>

namespace logicalaccess
{
void DESFireEV1NFCTag4CardService::createNFCApplication(
    unsigned int aid, std::shared_ptr<DESFireKey> masterPICCKey,
    unsigned short isoFIDApplication, unsigned short isoFIDCapabilityContainer,
    unsigned short isoFIDNDEFFile, unsigned short NDEFFileSize)
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));
    std::shared_ptr<DESFireEV1Commands> desfireev1command(
        std::dynamic_pointer_cast<DESFireEV1Commands>(getChip()->getCommands()));

    desfirecommand->selectApplication(0x00);
    if (masterPICCKey)
    {
        try
        {
            desfirecommand->authenticate(0x00, masterPICCKey);
        }
        catch (std::exception &)
        {
        }
    }

    unsigned char dfName[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};
    ByteVector dfNameVector(dfName, dfName + 7);
    desfireev1command->createApplication(aid, KS_DEFAULT, 1, DF_KEY_DES, FIDS_ISO_FID,
                                         isoFIDApplication, dfNameVector);

    std::shared_ptr<DESFireKey> key(
        new DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
    key->setKeyType(DF_KEY_DES);
    desfirecommand->selectApplication(aid);
    desfirecommand->authenticate(0x00, key);

    DESFireAccessRights dar;
    dar.changeAccess       = AR_KEY0;
    dar.readAccess         = AR_FREE;
    dar.readAndWriteAccess = AR_KEY0;
    dar.writeAccess        = AR_KEY0;

    desfireev1command->createStdDataFile(0x01, CM_PLAIN, dar, 0x0f,
                                         isoFIDCapabilityContainer);
    writeCapabilityContainer(isoFIDCapabilityContainer, isoFIDNDEFFile, NDEFFileSize);

    dar.changeAccess       = AR_KEY0;
    dar.readAccess         = AR_FREE;
    dar.readAndWriteAccess = AR_FREE;
    dar.writeAccess        = AR_FREE;

    desfireev1command->createStdDataFile(0x02, CM_PLAIN, dar, 0xff, isoFIDNDEFFile);
}

void DESFireEV1NFCTag4CardService::deleteNFCApplication(
    unsigned int aid, std::shared_ptr<DESFireKey> masterPICCKey) const
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));

    desfirecommand->selectApplication(0x00);
    if (masterPICCKey)
    {
        desfirecommand->authenticate(0x00, masterPICCKey);
    }
    desfirecommand->deleteApplication(aid);
}

std::shared_ptr<NdefMessage>
DESFireEV1NFCTag4CardService::readNDEFFile(const ByteVector &appDFName,
                                           unsigned short isoFIDNDEFFile)
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));

    desfirecommand->selectApplication(0x00);
    return ISO7816NFCTag4CardService::readNDEFFile(appDFName, isoFIDNDEFFile);
}

void DESFireEV1NFCTag4CardService::writeNDEF(std::shared_ptr<NdefMessage> records)
{
    std::shared_ptr<DESFireKey> picckey(
        new DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
    createNFCApplication(0x000001, picckey);
    writeNDEFFile(records);
}

void DESFireEV1NFCTag4CardService::eraseNDEF()
{
    std::shared_ptr<DESFireCommands> desfirecommand(
        std::dynamic_pointer_cast<DESFireCommands>(getChip()->getCommands()));
    desfirecommand->selectApplication(0);
    desfirecommand->authenticate(0);
    desfirecommand->erase();
}

std::shared_ptr<ISO7816Commands> DESFireEV1NFCTag4CardService::getISO7816Commands() const
{
    return getDESFireChip()->getDESFireEV1Commands()->getISO7816Commands();
}
}