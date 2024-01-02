/**
 * \file desfireev2commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV2 commands.
 */

#ifndef LOGICALACCESS_DESFIREEV2COMMANDS_HPP
#define LOGICALACCESS_DESFIREEV2COMMANDS_HPP

#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>

namespace logicalaccess
{
#ifndef DFEV2_INS
#define DFEV2_INS
#define DFEV2_INS_CHANGEKEY_EV2 0xC6
#define DFEV2_INS_AUTHENTICATE_EV2_FIRST 0x71
#define DFEV2_INS_INITIALIZEKEYSET 0x56
#define DFEV2_INS_ROLLKEYSET 0x55
#define DFEV2_INS_FINALIZEKEYSET 0x57
#define DFEV2_INS_CHANGEKEYEV2 0xc6
#define DFEV2_INS_CREATE_TRANSACTION_MAC_FILE 0xce
#define DFEV2_INS_CREATE_DELEGATED_APPLICATION 0xc9

// Prepare Proximity Check
#define DFEV2_INS_PREPARE_PC 0xF0
// ProximityCheck data exchange
#define DFEV2_INS_PROXIMITY_CHECK 0xF2
// ProximityCheck validation
#define DFEV2_INS_VERIFY_PC 0xFD

#define DFEV2_PROXIMITY_CHECK_KEY_NO 0x21

#endif

class DESFireEV2Chip;

/**
 * \brief The DESFire EV2 base commands class.
 */
class LLA_CARDS_DESFIRE_API DESFireEV2Commands : public ICommands
{
  public:
    virtual void changeKeyEV2(uint8_t keyset, uint8_t keyno,
                              std::shared_ptr<DESFireKey> key) = 0;

    virtual void authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key) = 0;

    virtual void sam_authenticateEV2First(uint8_t keyno,
                                          std::shared_ptr<DESFireKey> key) = 0;

    virtual void createApplication(
        unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) = 0;

    virtual void createDelegatedApplication(
        std::pair<ByteVector, ByteVector> damInfo, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) = 0;

    virtual std::pair<ByteVector, ByteVector> createDAMChallenge(
        std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
        std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) = 0;

    virtual void initliazeKeySet(uint8_t keySetNo, DESFireKeyType keySetType) = 0;

    virtual void rollKeySet(uint8_t keySetNo) = 0;

    virtual void finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion) = 0;

    virtual void createTransactionMACFile(unsigned char fileno,
                                          EncryptionMode comSettings,
                                          const DESFireAccessRights &accessRights,
                                          std::shared_ptr<DESFireKey> tmkey,
                                          unsigned char tmkversion) = 0;

    virtual ByteVector getKeyVersion(uint8_t keysetno, uint8_t keyno) = 0;

    virtual void setConfiguration(bool formatCardEnabled, bool randomIdEnabled,
                                  bool PCMandatoryEnabled,
                                  bool AuthVCMandatoryEnabled) = 0;

    virtual void setConfiguration(uint8_t sak1, uint8_t sak2) = 0;

    virtual void setConfiguration(bool D40SecureMessagingEnabled,
                                  bool EV1SecureMessagingEnabled,
                                  bool EV2ChainedWritingEnabled) = 0;

    virtual void setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5,
                                       uint8_t pdcap2_6) = 0;

    virtual void setConfiguration(ByteVector DAMMAC, ByteVector ISODFNameOrVCIID) = 0;

    virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                                    std::vector<DESFireAccessRights> accessRights,
                                    bool plain) = 0;

    /**
     * Perform proximity check.
     *
     * This encompass the 3 DESFire commands.
     *    + PreparePC
     *    + ProximityCheck
     *    + VerifyPC
     *
     * @param key The VCProximity key. If null, will try to get the corresponding
     * configured key from the associated crypto object.
     * @param chunk_size Number of byte to send in each ProximityCheck command.
     *        Since we send 8 bytes, this value must be either 1, 2 or 4.
     *        todo: EV2 supports any chunk size. Maybe we could accept a vector<int> defining each chunk size ?
     * // Todo maybe add a user-defined timing parameter.
     */
    virtual void proximityCheck(std::shared_ptr<DESFireKey> key, uint8_t chunk_size) = 0;

  private:
    std::shared_ptr<DESFireEV2Chip> getDESFireEV2Chip() const;
};
} // namespace logicalaccess

#endif /* LOGICALACCESS_DESFIREEV2COMMANDS_HPP */