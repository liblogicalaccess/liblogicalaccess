#include <logicalaccess/plugins/readers/iso7816/commands/desfireev3iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>

namespace logicalaccess
{

namespace
{

constexpr std::size_t UID_FORMAT_SIZE    = 1;
constexpr std::size_t UID_LENGTH_SIZE    = 1;
constexpr std::size_t UID_7_SIZE         = 7;
constexpr std::size_t UID_4_SIZE         = 4;
constexpr std::size_t UID_10_SIZE        = 10;
constexpr std::size_t BATCH_NO_SIZE      = 3;
constexpr std::size_t TYPE_ID_SIZE       = 2;
constexpr std::size_t CW_PROD_SIZE       = 1;
constexpr std::size_t YEAR_PROD_SIZE     = 1;
constexpr std::size_t FAB_ID_SIZE        = 1;
constexpr std::size_t MAC_SIZE           = 8;
constexpr std::size_t VERSION_FRAME_SIZE = 7;

constexpr std::size_t FIXED_MANUFACTURING_SIZE = BATCH_NO_SIZE + TYPE_ID_SIZE + CW_PROD_SIZE + YEAR_PROD_SIZE;

constexpr std::size_t UID_7_PAYLOAD_SIZE = UID_7_SIZE + FIXED_MANUFACTURING_SIZE;
constexpr std::size_t UID_4_PAYLOAD_SIZE = UID_FORMAT_SIZE + UID_LENGTH_SIZE + UID_4_SIZE + FIXED_MANUFACTURING_SIZE;
constexpr std::size_t UID_10_PAYLOAD_SIZE = UID_FORMAT_SIZE + UID_LENGTH_SIZE + UID_10_SIZE + FIXED_MANUFACTURING_SIZE;

constexpr std::size_t DUOX_UID_7_PAYLOAD_SIZE = UID_7_SIZE + FIXED_MANUFACTURING_SIZE + FAB_ID_SIZE;
constexpr std::size_t DUOX_UID_10_PAYLOAD_SIZE = UID_FORMAT_SIZE + UID_LENGTH_SIZE + UID_10_SIZE +
                                                 FIXED_MANUFACTURING_SIZE + FAB_ID_SIZE;

} // namespace

DESFireEV3ISO7816Commands::DESFireEV3ISO7816Commands()
    : DESFireEV2ISO7816Commands(CMD_DESFIREEV3ISO7816)
{
}

DESFireEV3ISO7816Commands::DESFireEV3ISO7816Commands(const std::string &cmd_type)
    : DESFireEV2ISO7816Commands(cmd_type)
{
}

std::shared_ptr<Chip> DESFireEV3ISO7816Commands::getChip() const
{
    return DESFireEV2ISO7816Commands::getChip();
}

std::shared_ptr<ReaderCardAdapter> DESFireEV3ISO7816Commands::getReaderCardAdapter() const
{
    return DESFireEV2ISO7816Commands::getReaderCardAdapter();
}

void DESFireEV3ISO7816Commands::changeKeyEV2(uint8_t keyset, uint8_t keyno,
                                             std::shared_ptr<DESFireKey> key)
{
    DESFireEV2ISO7816Commands::changeKeyEV2(keyset, keyno, key);
}

void DESFireEV3ISO7816Commands::authenticateEV2First(uint8_t keyno,
                                                     std::shared_ptr<DESFireKey> key)
{
    DESFireEV2ISO7816Commands::authenticateEV2First(keyno, key);
}

void DESFireEV3ISO7816Commands::sam_authenticateEV2First(uint8_t keyno,
                                                         std::shared_ptr<DESFireKey> key)
{
    DESFireEV2ISO7816Commands::sam_authenticateEV2First(keyno, key);
}

void DESFireEV3ISO7816Commands::createApplication(
    unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
    DESFireKeyType cryptoMethod, FidSupport fidSupported, unsigned short isoFID,
    ByteVector isoDFName, unsigned char numberKeySets, unsigned char maxKeySize,
    unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    DESFireEV2ISO7816Commands::createApplication(
        aid, settings, maxNbKeys, cryptoMethod, fidSupported, isoFID, isoDFName,
        numberKeySets, maxKeySize, actualkeySetVersion, rollkeyno, specificCapabilityData,
        specificVCKeys);
}

void DESFireEV3ISO7816Commands::createDelegatedApplication(
    std::pair<ByteVector, ByteVector> damInfo, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    DESFireEV2ISO7816Commands::createDelegatedApplication(
        damInfo, aid, DAMSlotNo, DAMSlotVersion, quotatLimit, settings, maxNbKeys,
        cryptoMethod, fidSupported, isoFID, isoDFName, numberKeySets, maxKeySize,
        actualkeySetVersion, rollkeyno, specificCapabilityData, specificVCKeys);
}

std::pair<ByteVector, ByteVector> DESFireEV3ISO7816Commands::createDAMChallenge(
    std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
    std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid, unsigned short DAMSlotNo,
    unsigned char DAMSlotVersion, unsigned short quotatLimit, DESFireKeySettings settings,
    unsigned char maxNbKeys, DESFireKeyType cryptoMethod, FidSupport fidSupported,
    unsigned short isoFID, ByteVector isoDFName, unsigned char numberKeySets,
    unsigned char maxKeySize, unsigned char actualkeySetVersion, unsigned char rollkeyno,
    bool specificCapabilityData, bool specificVCKeys)
{
    return DESFireEV2ISO7816Commands::createDAMChallenge(
        DAMMACKey, DAMENCKey, DAMDefaultKey, aid, DAMSlotNo, DAMSlotVersion, quotatLimit,
        settings, maxNbKeys, cryptoMethod, fidSupported, isoFID, isoDFName, numberKeySets,
        maxKeySize, actualkeySetVersion, rollkeyno, specificCapabilityData,
        specificVCKeys);
}

void DESFireEV3ISO7816Commands::initializeKeySet(uint8_t keySetNo,
                                                DESFireKeyType keySetType)
{
    DESFireEV2ISO7816Commands::initializeKeySet(keySetNo, keySetType);
}

void DESFireEV3ISO7816Commands::rollKeySet(uint8_t keySetNo)
{
    return DESFireEV2ISO7816Commands::rollKeySet(keySetNo);
}

void DESFireEV3ISO7816Commands::finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion)
{
    return DESFireEV2ISO7816Commands::finalizeKeySet(keySetNo, keySetVersion);
}

void DESFireEV3ISO7816Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights,
    unsigned int fileSize, unsigned short isoFID,
    bool multiAccessRights)
{
    DESFireEV2ISO7816Commands::createStdDataFile(fileno, comSettings, accessRights, fileSize, isoFID, multiAccessRights);
}

void DESFireEV3ISO7816Commands::createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights,
    unsigned int fileSize, unsigned short isoFID,
    bool multiAccessRights, bool sdmAndMirroring)
{
    
    int tmpComSettings = comSettings;
    if (multiAccessRights)
    {
        tmpComSettings |= 0x80; // Additional access rights
    }
    if (sdmAndMirroring)
    {
        tmpComSettings |= 0x40; // Secure Dynamic Messaging and Mirroring
    }
    
    comSettings = static_cast<EncryptionMode>(tmpComSettings);
    DESFireEV1ISO7816Commands::createStdDataFile(fileno, comSettings, accessRights,
                                                 fileSize, isoFID);
}

void DESFireEV3ISO7816Commands::createBackupFile(unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights,
    unsigned int fileSize, unsigned short isoFID,
    bool multiAccessRights)
{
    DESFireEV2ISO7816Commands::createBackupFile(fileno, comSettings, accessRights, fileSize, isoFID, multiAccessRights);
}

void DESFireEV3ISO7816Commands::createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights,
    unsigned int fileSize,
    unsigned int maxNumberOfRecords,
    unsigned short isoFID, bool multiAccessRights)
{
    DESFireEV2ISO7816Commands::createLinearRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, isoFID, multiAccessRights);
}

void DESFireEV3ISO7816Commands::createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights,
    unsigned int fileSize,
    unsigned int maxNumberOfRecords,
    unsigned short isoFID, bool multiAccessRights)
{
    DESFireEV2ISO7816Commands::createCyclicRecordFile(fileno, comSettings, accessRights, fileSize, maxNumberOfRecords, isoFID, multiAccessRights);
}

void DESFireEV3ISO7816Commands::createTransactionMACFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, std::shared_ptr<DESFireKey> tmkey)
{
    DESFireEV2ISO7816Commands::createTransactionMACFile(fileno, comSettings, accessRights, tmkey);
}

ByteVector DESFireEV3ISO7816Commands::getKeyVersion(uint8_t keysetno, uint8_t keyno)
{
    return DESFireEV2ISO7816Commands::getKeyVersion(keysetno, keyno);
}

void DESFireEV3ISO7816Commands::setConfiguration(bool formatCardEnabled,
                                                 bool randomIdEnabled,
                                                 bool PCMandatoryEnabled,
                                                 bool AuthVCMandatoryEnabled)
{
    DESFireEV2ISO7816Commands::setConfiguration(
        formatCardEnabled, randomIdEnabled, PCMandatoryEnabled, AuthVCMandatoryEnabled);
}

void DESFireEV3ISO7816Commands::setConfiguration(uint8_t sak1, uint8_t sak2)
{
    return DESFireEV2ISO7816Commands::setConfiguration(sak1, sak2);
}

void DESFireEV3ISO7816Commands::setConfiguration(bool D40SecureMessagingEnabled,
                                                 bool EV1SecureMessagingEnabled,
                                                 bool EV2ChainedWritingEnabled)
{
    DESFireEV2ISO7816Commands::setConfiguration(
        D40SecureMessagingEnabled, EV1SecureMessagingEnabled, EV2ChainedWritingEnabled);
}

void DESFireEV3ISO7816Commands::setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5,
                                                      uint8_t pdcap2_6)
{
    DESFireEV2ISO7816Commands::setConfigurationPDCap(pdcap1_2, pdcap2_5, pdcap2_6);
}

void DESFireEV3ISO7816Commands::setConfiguration(ByteVector DAMMAC,
                                                 ByteVector ISODFNameOrVCIID)
{
    DESFireEV2ISO7816Commands::setConfiguration(DAMMAC, ISODFNameOrVCIID);
}

void DESFireEV3ISO7816Commands::changeFileSettings(
    unsigned char fileno, EncryptionMode comSettings,
    std::vector<DESFireAccessRights> accessRights)
{
    DESFireEV2ISO7816Commands::changeFileSettings(fileno, comSettings, accessRights);
}

void DESFireEV3ISO7816Commands::changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
    std::vector<DESFireAccessRights> accessRights,
    bool sdmAndMirroring,
    unsigned int tmcLimit,
    bool sdmVCUID,
    bool sdmReadCtr,
    bool sdmReadCtrLimit,
    bool sdmEncFileData,
    bool asciiEncoding,
    DESFireAccessRights sdmAccessRights,
    unsigned int vcuidOffset,
    unsigned int sdmReadCtrOffset,
    unsigned int piccDataOffset,
    unsigned int sdmMacInputOffset,
    unsigned int sdmEncOffset,
    unsigned int sdmEncLength,
    unsigned int sdmMacOffset,
    unsigned int sdmReadCtrLimitValue)
{
    EXCEPTION_ASSERT_WITH_LOG(accessRights.size() > 0 && accessRights.size() <= 8,
                              LibLogicalAccessException,
                              "Invalid accessrights has been provided.");

    LOG(LogLevel::INFOS) << "changeFileSettings with parameters:" << " - "
                         << "fileno: " << fileno << " - "
                         << "comSettings: " << comSettings << " - "
                         << "accessRights: " << accessRights.size() << " - "
                         << "sdmAndMirroring: " << sdmAndMirroring << " - "
                         << "tmcLimit: " << tmcLimit << " - "
                         << "sdmVCUID: " << sdmVCUID << " - "
                         << "sdmReadCtr: " << sdmReadCtr << " - "
                         << "sdmReadCtrLimit: " << sdmReadCtrLimit << " - "
                         << "sdmEncFileData: " << sdmEncFileData << " - "
                         << "asciiEncoding: " << asciiEncoding << " - "
                         << "sdmAccessRights.SDMMetaRead: " << sdmAccessRights.readAccess << " - "
                         << "sdmAccessRights.SDMFileRead: " << sdmAccessRights.writeAccess << " - "
                         << "vcuidOffset: " << vcuidOffset << " - "
                         << "sdmReadCtrOffset: " << sdmReadCtrOffset << " - "
                         << "piccDataOffset: " << piccDataOffset << " - "
                         << "sdmMacInputOffset: " << sdmMacInputOffset << " - "
                         << "sdmEncOffset: " << sdmEncOffset << " - "
                         << "sdmEncLength: " << sdmEncLength << " - "
                         << "sdmMacOffset: " << sdmMacOffset << " - "
                         << "sdmReadCtrLimitValue: " << sdmReadCtrLimitValue;

    ByteVector command;
    unsigned char fileOpt = static_cast<unsigned char>(comSettings);

    short ar = AccessRightsInMemory(accessRights[0]);
    BufferHelper::setUShort(command, ar);
    if (accessRights.size() > 1)
    {
        fileOpt |= 0x80; // Multi access rights

        command.push_back(static_cast<unsigned char>(accessRights.size() - 1));
        for (int x = 1; x < static_cast<int>(accessRights.size()); ++x)
        {
            ar = AccessRightsInMemory(accessRights[x]);
            BufferHelper::setUShort(command, ar);
        }
    }

    if (sdmAndMirroring)
    {
        fileOpt |= 0x40;

        unsigned char sdmOpt = 0x00 |
            (sdmVCUID ? 0x80 : 0x00) |
            (sdmReadCtr ? 0x40 : 0x00) |
            (sdmReadCtrLimit ? 0x20 : 0x00) |
            (sdmEncFileData ? 0x10 : 0x00) |
            (asciiEncoding ? 0x01 : 0x00);
        command.push_back(sdmOpt);
        ar = AccessRightsInMemory(sdmAccessRights);
        BufferHelper::setUShort(command, ar);

        if (sdmVCUID && sdmAccessRights.readAccess == 0x0E) // SDMMetaRead
        {
            command.push_back(static_cast<unsigned char>(vcuidOffset & 0xff));
            command.push_back(static_cast<unsigned char>((vcuidOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((vcuidOffset & 0xffffff) >> 16));
        }
        if (sdmReadCtr && sdmAccessRights.readAccess == 0x0E) // SDMMetaRead
        {
            command.push_back(static_cast<unsigned char>(sdmReadCtrOffset & 0xff));
            command.push_back(static_cast<unsigned char>((sdmReadCtrOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmReadCtrOffset & 0xffffff) >> 16));
        }
        if (sdmAccessRights.readAccess != 0x0E && sdmAccessRights.readAccess != 0x0F) // SDMMetaRead
        {
            command.push_back(static_cast<unsigned char>(piccDataOffset & 0xff));
            command.push_back(static_cast<unsigned char>((piccDataOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((piccDataOffset & 0xffffff) >> 16));
        }
        if (sdmAccessRights.writeAccess != 0x0F) // SDMFileRead
        {
            command.push_back(static_cast<unsigned char>(sdmMacInputOffset & 0xff));
            command.push_back(static_cast<unsigned char>((sdmMacInputOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmMacInputOffset & 0xffffff) >> 16));
        }
        if (sdmEncFileData && sdmAccessRights.writeAccess != 0x0F) // SDMFileRead
        {
            command.push_back(static_cast<unsigned char>(sdmEncOffset & 0xff));
            command.push_back(static_cast<unsigned char>((sdmEncOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmEncOffset & 0xffffff) >> 16));
            command.push_back(static_cast<unsigned char>(sdmEncLength & 0xff));
            command.push_back(static_cast<unsigned char>((sdmEncLength & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmEncLength & 0xffffff) >> 16));
        }
        if (sdmAccessRights.writeAccess != 0x0F) // SDMFileRead
        {
            command.push_back(static_cast<unsigned char>(sdmMacOffset & 0xff));
            command.push_back(static_cast<unsigned char>((sdmMacOffset & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmMacOffset & 0xffffff) >> 16));
        }
        if (sdmReadCtrLimit)
        {
            command.push_back(static_cast<unsigned char>(sdmReadCtrLimitValue & 0xff));
            command.push_back(static_cast<unsigned char>((sdmReadCtrLimitValue & 0xffff) >> 8));
            command.push_back(static_cast<unsigned char>((sdmReadCtrLimitValue & 0xffffff) >> 16));
        }
    }

    if (tmcLimit > 0)
    {
        fileOpt |= 0x20;

        command.push_back(static_cast<unsigned char>(tmcLimit & 0xff));
        command.push_back(static_cast<unsigned char>((tmcLimit & 0xffff) >> 8));
        command.push_back(static_cast<unsigned char>((tmcLimit & 0xffffff) >> 16));
        command.push_back(static_cast<unsigned char>((tmcLimit & 0xffffffff) >> 24));
    }

    command.insert(command.begin(), fileOpt);
    
    ByteVector param;
    param.push_back(static_cast<unsigned char>(fileno));

    transmit_full(DF_INS_CHANGE_FILE_SETTINGS, command, param).getData();
}

void DESFireEV3ISO7816Commands::proximityCheck(std::shared_ptr<DESFireKey> key,
                                               uint8_t chunk_size)
{
    DESFireEV2ISO7816Commands::proximityCheck(key, chunk_size);
}

ByteVector DESFireEV3ISO7816Commands::commitTransaction(bool return_tmac)
{
    return DESFireEV2ISO7816Commands::commitTransaction(return_tmac);
}

ByteVector DESFireEV3ISO7816Commands::commitReaderID(ByteVector readerid)
{
    return DESFireEV2ISO7816Commands::commitReaderID(readerid);
}

void DESFireEV3ISO7816Commands::restoreTransfer(unsigned char target_fileno, unsigned char source_fileno)
{
    DESFireEV2ISO7816Commands::restoreTransfer(target_fileno, source_fileno);
}

ByteVector DESFireEV3ISO7816Commands::readSignature(unsigned char address)
{
    return DESFireEV2ISO7816Commands::readSignature(address);
}

bool DESFireEV3ISO7816Commands::performECCOriginalityCheck()
{
    return DESFireEV2ISO7816Commands::performECCOriginalityCheck();
}

ByteVector DESFireEV3ISO7816Commands::getFileCounters(unsigned char fileno)
{
    ByteVector params;
    params.push_back(fileno);
    return transmit_full(DFEV3_INS_GET_FILE_COUNTERS, {}, params).getData();
}

bool DESFireEV3ISO7816Commands::performAESOriginalityCheck()
{
    EXCEPTION_ASSERT_WITH_LOG(getSAMChip() != nullptr && getSAMChip()->getCardType() == "SAM_AV3",
                              LibLogicalAccessException,
                              "SAM AV3 is needed for AES Originality Check");

    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(DF_KEY_AES);
    key->setKeyVersion(0);
    auto ks = std::make_shared<SAMKeyStorage>();
    ks->setKeySlot(1);
    ks->setIsAbsolute(false);
    key->setKeyStorage(ks);

    authenticateEV2First(1, key);

    return true;
}

void DESFireEV3ISO7816Commands::validateAid(unsigned int aid)
{
    if (aid > MAX_AID)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DESFire EV3 application identifier.");
    }
}

// TODO Handle TransactionMAC cases and replace throw by log (command is merely rejected and PICC level is selected)
void DESFireEV3ISO7816Commands::validateApplicationPair(unsigned int aid1, unsigned int aid2)
{
    validateAid(aid1);
    validateAid(aid2);

    if (aid1 == PICC_AID || aid2 == PICC_AID)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "The PICC application cannot be selected together with another application.");

    if (aid1 == aid2)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Selected DESFire EV3 applications must have different application identifiers.");
}

void DESFireEV3ISO7816Commands::selectApplication(unsigned int aid)
{
    validateAid(aid); // TODO should select PICC on any rejected case
    DESFireISO7816Commands::selectApplication(aid);
    // TODO if transmit not successful, we should do :
    // if (result.getSW1() != 0x91 && result.getSW2() != 0x00) {
    // getDESFireChip()->getCrypto()->selectApplication(PICC_AID);
    // THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFire EV3 SelectApplication failed."); }
}

void DESFireEV3ISO7816Commands::selectApplication(unsigned int aid1, unsigned int aid2)
{
    validateApplicationPair(aid1, aid2); // TODO should select PICC on any rejected case

    ByteVector command;
    command.reserve(6);

    ByteVector aid;
    DESFireLocation::convertUIntToAid(aid1, aid);
    command.insert(command.end(), aid.begin(), aid.end());

    aid.clear();
    DESFireLocation::convertUIntToAid(aid2, aid);
    command.insert(command.end(), aid.begin(), aid.end());

    const auto result = transmit(DF_INS_SELECT_APPLICATION, command);

    // TODO most of the errors are caught at resultchecker level
    if (result.getSW1() != 0x91 && result.getSW2() != 0x00)
    {
        getDESFireChip()->getCrypto()->selectApplication(PICC_AID);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "DESFire EV3 SelectApplication failed.");
    }

    getDESFireChip()->getCrypto()->selectApplication(aid1);
}

DESFireCommands::DESFireGetVersionOption DESFireEV3ISO7816Commands::getVersionOption() const
{
    return DESFireGetVersionOption::None;
}

bool DESFireEV3ISO7816Commands::supportsFourByteUID() const
{
    return true;
}

DESFireCommands::DESFireCardVersion DESFireEV3ISO7816Commands::getVersionImpl(DESFireGetVersionOption option,
                                          bool allowFourByteUID)
{
    DESFireCardVersion dataVersion{};
    ByteVector allresult;

    const std::shared_ptr<DESFireCrypto> crypto = getDESFireChip()->getCrypto();

    auto result = transmit_nomacv(DF_INS_GET_VERSION);

    if (result.getData().size() != VERSION_FRAME_SIZE || result.getSW2() != DF_INS_ADDITIONAL_FRAME)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DESFire GetVersion hardware response.");

    dataVersion.hardware = parseVersionInfo(result.getData());
    allresult.insert(allresult.end(), result.getData().begin(), result.getData().end());

    result = transmit_plain(DF_INS_ADDITIONAL_FRAME);

    if (result.getData().size() != VERSION_FRAME_SIZE || result.getSW2() != DF_INS_ADDITIONAL_FRAME)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DESFire GetVersion software response.");

    dataVersion.software = parseVersionInfo(result.getData());
    allresult.insert(allresult.end(), result.getData().begin(), result.getData().end());

    result = transmit_plain(DF_INS_ADDITIONAL_FRAME);

    if (result.getSW2() != 0x00)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Invalid DESFire GetVersion manufacturing response status.");

    const ByteVector &manufacturingData = result.getData();
    dataVersion.manufacturing = parseManufacturingInfo(manufacturingData, option, allowFourByteUID);

    dataVersion.hasFabID = false;
    dataVersion.fabID    = 0;

    allresult.insert(allresult.end(), manufacturingData.begin(), manufacturingData.end());

    if (option == DESFireGetVersionOption::FabID)
    {
        const std::size_t payloadSize = getManufacturingPayloadSize(manufacturingData, option, allowFourByteUID);

        if (manufacturingData.size() > payloadSize)
        {
            dataVersion.hasFabID = true;
            dataVersion.fabID    = manufacturingData[payloadSize];
        }
    }

    if (crypto->d_auth_method != CM_LEGACY)
    {
        if (!crypto->verifyMAC(true, allresult))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC verification failed.");
    }

    return dataVersion;
}

DESFireCommands::DESFireVersionInfo DESFireEV3ISO7816Commands::parseVersionInfo(const ByteVector &data)
{
    if (data.size() != VERSION_FRAME_SIZE)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Invalid DESFire GetVersion response : version frame size is invalid.");

    DESFireVersionInfo result{};
    result.vendor       = data[0];
    result.type         = data[1];
    result.subtype      = data[2];
    result.majorVersion = data[3];
    result.minorVersion = data[4];
    result.storageSize  = data[5];
    result.protocol     = data[6];

    return result;
}

DESFireCommands::DESFireManufacturingInfo DESFireEV3ISO7816Commands::parseManufacturingInfo(const ByteVector &data,
                                                  DESFireGetVersionOption option, bool allowFourByteUID)
{
    const std::size_t payloadSize = getManufacturingPayloadSize(data, option, allowFourByteUID);

    std::size_t offset    = 0;
    std::size_t uidLength = UID_7_SIZE;

    const bool hasUIDDescriptor = payloadSize != UID_7_PAYLOAD_SIZE && payloadSize != DUOX_UID_7_PAYLOAD_SIZE;

    if (hasUIDDescriptor)
    {
        ensureSize(data, offset, UID_FORMAT_SIZE + UID_LENGTH_SIZE,
                   "Invalid DESFire GetVersion response : incomplete UID descriptor.");

        const unsigned char uidFormat         = data[offset];
        const unsigned char declaredUidLength = data[offset + 1];

        if (uidFormat != 0x00)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "Invalid DESFire GetVersion response : unsupported UID format.");

        if (declaredUidLength != UID_4_SIZE && declaredUidLength != UID_10_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "Invalid DESFire GetVersion response : invalid UID length.");

        if (!allowFourByteUID && declaredUidLength == UID_4_SIZE)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                "Invalid DESFire GetVersion response : 4-byte UID is not supported.");

        uidLength = declaredUidLength;
        offset += UID_FORMAT_SIZE + UID_LENGTH_SIZE;
    }

    ensureSize(data, offset, uidLength + FIXED_MANUFACTURING_SIZE,
               "Invalid DESFire GetVersion response : incomplete manufacturing data.");

    DESFireManufacturingInfo result{};

    result.uid.assign(data.begin() + offset, data.begin() + offset + uidLength);
    offset += uidLength;

    result.batchNo.assign(data.begin() + offset, data.begin() + offset + BATCH_NO_SIZE);
    offset += BATCH_NO_SIZE;

    result.typeId.assign(data.begin() + offset, data.begin() + offset + TYPE_ID_SIZE);
    offset += TYPE_ID_SIZE;

    result.cwProd   = data[offset++];
    result.yearProd = data[offset++];

    if (offset != payloadSize)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
            "Invalid DESFire GetVersion response : unexpected manufacturing data.");

    return result;
}

std::size_t DESFireEV3ISO7816Commands::getManufacturingPayloadSize(
    const ByteVector &data, DESFireGetVersionOption option, bool allowFourByteUID)
{
    if (isValidManufacturingPayloadSize(data.size(), option, allowFourByteUID))
        return data.size();

    if (data.size() >= MAC_SIZE && isValidManufacturingPayloadSize(data.size() - MAC_SIZE, option, allowFourByteUID))
        return data.size() - MAC_SIZE;

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
        "Invalid DESFire GetVersion response : invalid manufacturing data length.");
}

void DESFireEV3ISO7816Commands::ensureSize(const ByteVector &data, std::size_t offset, std::size_t required,
                const char *message)
{
    if (offset > data.size() || required > data.size() - offset)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, message);
}

bool DESFireEV3ISO7816Commands::isValidManufacturingPayloadSize(
    std::size_t size, DESFireGetVersionOption option, bool allowFourByteUID)
{
    if (option == DESFireGetVersionOption::FabID)
        return size == DUOX_UID_7_PAYLOAD_SIZE || size == DUOX_UID_10_PAYLOAD_SIZE;

    if (allowFourByteUID)
        return size == UID_4_PAYLOAD_SIZE || size == UID_7_PAYLOAD_SIZE || size == UID_10_PAYLOAD_SIZE;

    return size == UID_7_PAYLOAD_SIZE || size == UID_10_PAYLOAD_SIZE;
}

DESFireCommands::DESFireCardVersion DESFireEV3ISO7816Commands::getVersion()
{
    return getVersionImpl(DESFireGetVersionOption::None, true);
}

}
