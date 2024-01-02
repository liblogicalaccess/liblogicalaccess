#include <logicalaccess/plugins/readers/iso7816/commands/desfireev3iso7816commands.hpp>

namespace logicalaccess
{

DESFireEV3ISO7816Commands::DESFireEV3ISO7816Commands()
    : DESFireEV2ISO7816Commands(CMD_DESFIREEV3ISO7816)
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

void DESFireEV3ISO7816Commands::initliazeKeySet(uint8_t keySetNo,
                                                DESFireKeyType keySetType)
{
    DESFireEV2ISO7816Commands::initliazeKeySet(keySetNo, keySetType);
}

void DESFireEV3ISO7816Commands::rollKeySet(uint8_t keySetNo)
{
    return DESFireEV2ISO7816Commands::rollKeySet(keySetNo);
}

void DESFireEV3ISO7816Commands::finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion)
{
    return DESFireEV2ISO7816Commands::finalizeKeySet(keySetNo, keySetVersion);
}

void DESFireEV3ISO7816Commands::createTransactionMACFile(
    unsigned char fileno, EncryptionMode comSettings,
    const DESFireAccessRights &accessRights, std::shared_ptr<DESFireKey> tmkey,
    unsigned char tmkversion)
{
    DESFireEV2ISO7816Commands::createTransactionMACFile(fileno, comSettings, accessRights,
                                                        tmkey, tmkversion);
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
    std::vector<DESFireAccessRights> accessRights, bool plain)
{
    DESFireEV2ISO7816Commands::changeFileSettings(fileno, comSettings, accessRights,
                                                  plain);
}

void DESFireEV3ISO7816Commands::proximityCheck(std::shared_ptr<DESFireKey> key,
                                               uint8_t chunk_size)
{
    return DESFireEV2ISO7816Commands::proximityCheck(key, chunk_size);
}
}
