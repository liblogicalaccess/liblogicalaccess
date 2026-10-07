#pragma once

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev3iso7816commands.hpp>
#include <logicalaccess/plugins/cards/desfire/duoxcommands.hpp>

// TODO move it later inside duoxcrypto file
#include <utility>

namespace logicalaccess
{

// TODO Move this structure to a more appropriate location; no need to do it now, move it when refactoring
struct DUOXECCAuthenticationState
{
    bool active = false;

    CurveID curve = CurveID::NIST_P256;

    bool mutualAuthentication = false;
    bool certificatePresent   = false;

    std::uint8_t caRootKeyNo          = 0;
    std::uint8_t secondaryCaRootKeyNo = 0;
    std::uint8_t certFileNo           = 0;
    std::uint8_t privateKeyNo         = 0;

    ByteVector optsA;

    /*
     * Ephemeral ECDH key material.
     *
     * The private key must remain local and must be erased when the authentication context is destroyed or reset
     */
    ByteVector ephemeralPrivateKey;

    ECPoint ephemeralPublicA;
    ECPoint ephemeralPublicB;

    /*
     * Authentication session keys
     */
    SessionKeys sessionKeys;

    // TODO EV2/secure-messaging context will be added here once DUOX post-authentication secure messaging format is wired
};

/**
 * \brief ISO/IEC 7816 command implementation for DUOX cards
 *
 * DUOX commands are based on the DESFire EV3 command set and use the DESFire EV3 ISO/IEC 7816 transport implementation
 *
 * Multiple inheritance is intentional here :
 * - DESFireEV3ISO7816Commands provides the ISO/IEC 7816 implementation
 * - DUOXCommands provides the DUOX command interface
 *
 * The DESFire EV2/EV3 methods are explicitly forwarded to DESFireEV3ISO7816Commands
 * to resolve the duplicated DESFire command hierarchy introduced by the multiple inheritance
 */
#define CMD_DUOXISO7816 "DUOXISO7816"

class LLA_READERS_ISO7816_API DUOXISO7816Commands : public DESFireEV3ISO7816Commands,
                                                    public DUOXCommands
{
  public:
    /**
     * \brief Constructs DUOX ISO/IEC 7816 commands
     *
     * Initializes the underlying DESFire EV3 ISO/IEC 7816 implementation with the DUOX command type
     */
    DUOXISO7816Commands();

    /**
     * \brief Destructor
     */
    ~DUOXISO7816Commands() override = default;

    /**
     * \brief Get the DUOX chip associated with these commands
     *
     * \return The associated DUOX chip
     */
    std::shared_ptr<Chip> getChip() const override;

    /**
     * \brief Get the reader/card adapter used by these commands
     *
     * \return The reader/card adapter
     */
    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override;

    /**
     * \brief Manage a DUOX asymmetric private key entry.
     *
     * Implements the DUOX ManageKeyPair command (0x46).
     */
    ByteVector manageKeyPair(std::uint8_t keyNo, DUOXManageKeyPairOption option,
                             CurveID curveId, std::uint16_t keyPolicy,
                             std::uint8_t writeAccess, std::uint32_t kucLimit,
                             const ByteVector &privateKey = ByteVector(),
                             DUOXCommunicationMode commMode = DUOXCommunicationMode::Full) override;

    void manageCARootKey(std::uint8_t keyNo, CurveID curveId,
                         std::uint16_t accessRights, std::uint8_t writeAccess,
                         std::uint8_t readAccess, std::uint8_t crlFile,
                         std::uint32_t crlFileAid, const ByteVector &publicKey,
                         const ByteVector &issuer, DUOXCommunicationMode commMode = DUOXCommunicationMode::Full) override;

    ByteVector exportKey(std::uint8_t keyNo, DUOXCommunicationMode commMode = DUOXCommunicationMode::Full) override;

    void authenticateEV2NonFirst(std::uint8_t keyno, std::shared_ptr<DESFireKey> currentKey = nullptr) override;

    std::uint32_t freeMem() override;

    DUOXKeySettings getKeySettings(DUOXKeySettingsOption option = DUOXKeySettingsOption::KeySettings) override;

    void changeKey(std::uint8_t keyNo, std::shared_ptr<DESFireKey> newKey) override;
    void changeKeyEV2(std::uint8_t keySetNo, std::uint8_t keyNo, std::shared_ptr<DESFireKey> newKey) override;

    /*
     * DESFire EV2 command interface.
     *
     * Explicit forwarding is required because DUOXCommands and
     * DESFireEV3ISO7816Commands both inherit from the DESFire command hierarchy
     * The ISO/IEC 7816 implementation remains the actual implementation
     */
    //void changeKeyEV2(uint8_t keyset, uint8_t keyno,
    //                  std::shared_ptr<DESFireKey> key) override;

    /*
     * DESFire EV2 command interface.
     *
     * This overload is retained because the inherited DESFire API identifies the complete KeyNo in one byte
     */
    void authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key) override;

    /**
     * \brief DUOX-specific AuthenticateEV2First entry point.
     *
     * Complete KeyNo is constructed rather than requiring callers to manipulate protocol bit fields themselves.
     *
     * \param targetedKeyNo Target authentication key number, six bit value.
     * \param key Authentication key.
     * \param secondaryApplication Whether the secondary-application bit is set.
     * \param pcdCap2 PCD capabilities. At most six bytes are sent.
     */
    void authenticateEV2First(std::uint8_t targetedKeyNo, std::shared_ptr<DESFireKey> key,
                              bool secondaryApplication = false,
                              ByteVector pcdCap2        = ByteVector()); // TODO : PCDcap2 may become persistent class state

    void sam_authenticateEV2First(uint8_t keyno, std::shared_ptr<DESFireKey> key) override;

    void createApplication(
        unsigned int aid, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    void createDelegatedApplication(
        std::pair<ByteVector, ByteVector> damInfo, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    std::pair<ByteVector, ByteVector> createDAMChallenge(
        std::shared_ptr<DESFireKey> DAMMACKey, std::shared_ptr<DESFireKey> DAMENCKey,
        std::shared_ptr<DESFireKey> DAMDefaultKey, unsigned int aid,
        unsigned short DAMSlotNo, unsigned char DAMSlotVersion,
        unsigned short quotatLimit, DESFireKeySettings settings, unsigned char maxNbKeys,
        DESFireKeyType cryptoMethod, FidSupport fidSupported = FIDS_NO_ISO_FID,
        unsigned short isoFID = 0x00, ByteVector isoDFName = ByteVector(),
        unsigned char numberKeySets = 0, unsigned char maxKeySize = 0,
        unsigned char actualkeySetVersion = 0, unsigned char rollkeyno = 0,
        bool specificCapabilityData = false, bool specificVCKeys = false) override;

    void initializeKeySet(uint8_t keySetNo, DESFireKeyType keySetType) override;

    void rollKeySet(uint8_t keySetNo) override;

    void finalizeKeySet(uint8_t keySetNo, uint8_t keySetVersion) override;

    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                           const DESFireAccessRights &accessRights, unsigned int fileSize,
                           unsigned short isoFID, bool multiAccessRights) override;

    void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                          const DESFireAccessRights &accessRights, unsigned int fileSize,
                          unsigned short isoFID, bool multiAccessRights) override;

    void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize, unsigned int maxNumberOfRecords,
                                unsigned short isoFID, bool multiAccessRights) override;

    void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize, unsigned int maxNumberOfRecords,
                                unsigned short isoFID, bool multiAccessRights) override;

    void createTransactionMACFile(unsigned char fileno, EncryptionMode comSettings,
                                  const DESFireAccessRights &accessRights,
                                  std::shared_ptr<DESFireKey> tmkey) override;

    ByteVector getKeyVersion(uint8_t keysetno, uint8_t keyno) override;

    void setConfiguration(bool formatCardEnabled, bool randomIdEnabled,
                          bool PCMandatoryEnabled, bool AuthVCMandatoryEnabled) override;

    void setConfiguration(uint8_t sak1, uint8_t sak2) override;

    void setConfiguration(bool D40SecureMessagingEnabled, bool EV1SecureMessagingEnabled,
                          bool EV2ChainedWritingEnabled) override;

    void setConfigurationPDCap(uint8_t pdcap1_2, uint8_t pdcap2_5,
                               uint8_t pdcap2_6) override;

    void setConfiguration(ByteVector DAMMAC, ByteVector ISODFNameOrVCIID) override;

    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            std::vector<DESFireAccessRights> accessRights) override;

    void proximityCheck(std::shared_ptr<DESFireKey> key, uint8_t chunk_size) override;

    ByteVector commitTransaction(bool return_tmac) override;
    ByteVector commitReaderID(ByteVector readerid) override;

    void restoreTransfer(unsigned char target_fileno, unsigned char source_fileno) override;

    ByteVector readSignature(unsigned char address = 0x00) override;

    bool performECCOriginalityCheck() override;

    /*
     * DESFire EV3 command interface.
     *
     * Explicitly forwarded to the ISO/IEC 7816 implementation to resolve the duplicated DESFireEV3Commands base
     */
    ByteVector getFileCounters(unsigned char fileno) override;

    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize,
                                   unsigned short isoFID,
                                   bool multiAccessRights, bool sdmAndMirroring) override;

    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
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
                                    unsigned int sdmReadCtrLimitValue) override;

    bool performAESOriginalityCheck() override;

    void selectApplication(unsigned int aid) override;
    void selectApplication(unsigned int aid1, unsigned int aid2) override;

    protected:
    ISO7816Response transmitDUOX(std::uint8_t cmd, const ByteVector &params, const ByteVector &data,
                 DUOXCommunicationMode commMode = DUOXCommunicationMode::Full);

    private:
    void changeKeyEV2Internal(std::uint8_t keySetNo, std::uint8_t keyNo,
        std::shared_ptr<DESFireKey> newKey, DUOXChangeKeyCommand commandType);




    // TODO move these functions into main class definition later (access specifiers redefined temporarily for organization)
    // And move ISO functions to ISO class but keep them here for now

    public:
    void isoGeneralAuthenticate(std::uint8_t caRootKeyNo,
                                std::uint8_t secondaryCaRootKeyNo, CurveID curve,
                                bool mutualAuthentication, bool certificatePresent,
                                std::uint8_t certFileNo, std::uint8_t privateKeyNo,
                                const ByteVector &privateKey,
                                const ByteVector &certificate) override;

      void isoGeneralAuthenticatePart1(std::uint8_t caRootKeyNo,
                                       std::uint8_t secondaryCaRootKeyNo,
                                       CurveID curve, bool mutualAuthentication,
                                       bool certificatePresent, std::uint8_t certFileNo,
                                       std::uint8_t privateKeyNo) override;

      void isoGeneralAuthenticatePart2(const ByteVector &privateKey, const ByteVector &certificate) override;

      ByteVector isoInternalAuthenticate(unsigned char keyno, const ByteVector &rndA,
                                              const ByteVector &optsA = ByteVector()) override;


      ByteVector isoSelectFile(std::uint8_t p1, std::uint8_t p2, const ByteVector &data, std::uint32_t le) override;
      ByteVector isoSelectFile(std::uint8_t p1, std::uint8_t p2, const ByteVector &data) override;
      ByteVector isoSelectFile(const ByteVector &dfName, bool returnFCI = true) override;
      ByteVector isoSelectApplicationWithECCFCI(const ByteVector &dfName, std::uint32_t le) override;
      ByteVector isoSelectApplicationForFastUID(const ByteVector &dfName, std::uint32_t le) override;

      ByteVector isoGeneralAuthenticateFinal(std::uint8_t caRootKeyNo, AuthMethod authMethod,
          const ByteVector &certA, std::uint8_t certFileNo, std::uint8_t privateKeyNo, const ByteVector &ePublicA,
          const ByteVector &msgAEnc, CurveID curve, std::uint32_t le) override;

      DESFireCardVersion getVersion() override;

    private:
    ISO7816Response sendDUOXGeneralAuthenticate(std::uint8_t p2, const ByteVector &data, std::uint16_t le);

    static std::uint8_t buildDUOXGeneralAuthenticateP2(std::uint8_t caRootKeyNo,
                                                       std::uint8_t secondaryCaRootKeyNo,
                                                       bool multipleApplicationSelection);

    static ByteVector buildOptsA(bool mutualAuthentication, bool certificateAIncluded,
                                     std::uint8_t certificateFileNo,
                                     std::uint8_t privateKeyNo);

    static ByteVector buildAuthenticationData(const ByteVector &ephemeralPublicKey);

    static ECPoint parseAuthenticationResponse(const ByteVector &response, CurveID curve);

    ByteVector parseGeneralAuthenticateFinalResponse(const ByteVector &data);

    DUOXECCAuthenticationState d_duoxECCAuthentication;

    // ===========================================
    // DUOX EV2 authentication helpers
    // ===========================================

    // TODO Move all of this later inside duoxcrypto file

    /**
     * \brief Derive the DUOX EV2 authentication session keys.
     *
     * DUOX extends the EV2 KDF to support AES-256 keys.
     * AES-128 produces one 16 byte CMAC per session key while AES-256 produces two 16-byte CMAC blocks per session key.
     *
     * \param key The diversified authentication key.
     * \param rndA The PCD-generated authentication challenge.
     * \param rndB The PICC-generated authentication challenge.
     */
    static std::pair<ByteVector, ByteVector>
    deriveEV2SessionKeys(const ByteVector &key, const ByteVector &rndA, const ByteVector &rndB);

    /**
     * \brief Rotate a 16-byte authentication challenge left by one byte.
     */
    static ByteVector rotateAuthenticationChallenge(const ByteVector &rnd);

    /**
     * \brief Validate the decrypted AuthenticateEV2FirstPart2 response.
     *
     * \param response Decrypted PICC response.
     * \param rndA Original PCD challenge.
     * \param expectedPCDCap2 PCD capabilities sent during Part 1.
     *
     * \return Transaction identifier.
     */
    static ByteVector validateEV2Part2Response(const ByteVector &response,
                                                   const ByteVector &rndA,
                                                   const ByteVector &expectedPCDCap2);

    /**
     * \brief Build an EV2 session-vector context from RndA and RndB.
     */
    static ByteVector buildEV2SessionContext(const ByteVector &rndA, const ByteVector &rndB);

    /**
     * \brief Build one DUOX EV2 KDF session vector.
     *
     * \param label First two bytes of the KDF label.
     * \param counter KDF counter.
     * \param length KDF output length field.
     * \param context 26-byte authentication context.
     */
    static ByteVector buildEV2SessionVector(std::uint8_t label0, std::uint8_t label1,
                                                std::uint16_t counter,
                                                std::uint16_t length,
                                                const ByteVector &context);

    /**
     * \brief Construct the protocol KeyNo byte.
     */
    static std::uint8_t buildEV2KeyNo(std::uint8_t targetedKeyNo, bool secondaryApplication);


};

} // namespace logicalaccess