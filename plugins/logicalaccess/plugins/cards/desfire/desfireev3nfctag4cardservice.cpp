/**
 * \file desfireev3nfctag4cardservice.cpp
 * \author Maxime C. <adrien-dev@leosac.com>
 * \brief DESFire EV3 NFC Tag type 4 CardService.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev3nfctag4cardservice.hpp>

namespace logicalaccess
{
void DESFireEV3NFCTag4CardService::writeSDM(const std::string& baseUri)
{
    createNFCApplication(0x000001);
    writeSDMFile(baseUri);
}

void DESFireEV3NFCTag4CardService::createNDEFFile(const DESFireAccessRights& dar, unsigned short isoFIDNDEFFile)
{
    auto df3cmd = std::dynamic_pointer_cast<DESFireEV3Commands>(getChip()->getCommands());
    df3cmd->createStdDataFile(0x02, CM_PLAIN, dar, 0xff, isoFIDNDEFFile, false, true);
}
    
void DESFireEV3NFCTag4CardService::writeSDMFile(const std::string& baseUri,
                                            const std::string& paramVcuid,
                                            const std::string& paramPicc,
                                            const std::string& paramReadCtr,
                                            const std::string& paramMAC,
                                            unsigned short isoFIDNDEFFile)
{
    auto uri = baseUri;
    if (uri.find("?") == std::string::npos)
    {
        uri += "?";
    }

    unsigned int recordOffset = 7;
    size_t vcuidOffset = 0;
    if (paramVcuid.size() > 0)
    {
        vcuidOffset = recordOffset + uri.size() + paramVcuid.size() + 1;
        uri += paramVcuid + "=00000000000000&";
    }
    size_t piccOffset = 0;
    if (paramPicc.size() > 0)
    {
        piccOffset = recordOffset + uri.size() + paramPicc.size() + 1;
        uri += paramPicc + "=00000000000000000000000000000000&";
    }
    size_t readCtrOffset = 0;
    if (paramReadCtr.size() > 0)
    {
        readCtrOffset = recordOffset + uri.size() + paramReadCtr.size() + 1;
        uri += paramReadCtr + "=000000&";
    }
    size_t macOffset = 0;
    if (paramMAC.size() > 0)
    {
        macOffset = recordOffset + uri.size() + paramMAC.size() + 1;
        uri += paramMAC + "=0000000000000000";
    }

    auto ndefmsg = std::make_shared<NdefMessage>();
    ndefmsg->addUriRecord(uri, NO_PREFIX);

    auto dfcmd = getDESFireEV3Chip()->getDESFireCommands();
    auto df3cmd = getDESFireEV3Chip()->getDESFireEV3Commands();

    logicalaccess::DESFireAccessRights dar;
    dar.changeAccess       = AR_KEY0;
    dar.readAccess         = AR_FREE;
    dar.readAndWriteAccess = AR_FREE;
    dar.writeAccess        = AR_FREE;

    logicalaccess::DESFireAccessRights sdmar;
    sdmar.readAccess = piccOffset > 0 ? logicalaccess::AR_KEY2 :
                       (vcuidOffset > 0 || readCtrOffset > 0 ? logicalaccess::AR_FREE : logicalaccess::AR_NEVER); // SDMMetaRead, Plain PICCData mirroring
    sdmar.writeAccess = logicalaccess::AR_KEY1; // SDMFileRead, 
    sdmar.readAndWriteAccess = logicalaccess::AR_NEVER; // RFU
    sdmar.changeAccess = logicalaccess::AR_FREE; // SDMCtrRet

    writeNDEFFile(ndefmsg, isoFIDNDEFFile);

    dfcmd->authenticate(0, d_app_new_key ? d_app_new_key : d_app_empty_key);
    df3cmd->changeFileSettings(2, logicalaccess::CM_PLAIN, { dar }, true, 0, vcuidOffset > 0 || piccOffset > 0, readCtrOffset > 0 || piccOffset > 0, false, false, true, sdmar, piccOffset == 0 ? vcuidOffset : 0, piccOffset == 0 ? readCtrOffset : 0, piccOffset, recordOffset, 0, 0, macOffset, 0);
}
}