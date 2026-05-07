/**
 * \file SAMAV2ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include <logicalaccess/plugins/cards/samav/samcommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav1iso7816commands.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samav2commands.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{

namespace sam
{
constexpr std::size_t MAX_APDU_DATA_SIZE = 0xFF;
enum class HostMode : unsigned char
{
    Plain       = 0x00,
    MAC         = 0x01,
    FullProtect = 0x02
};

constexpr unsigned char toByte(HostMode mode)
{
    return static_cast<unsigned char>(mode);
}
}

#define AV2_HEADER_LENGTH 0x05
#define AV2_HEADER_LENGTH_WITH_LE 0x06
#define AV2_LC_POS 0x04
#define CMD_SAMAV2ISO7816 "SAMAV2ISO7816"

#ifdef SWIG
%template(SAMISO7816KeyEntrySETAV2Commands)
        SAMISO7816Commands<KeyEntryAV2Information, SETAV2>;
#endif

/**
 * \brief The SAM AV2 base commands class.
 */
class LLA_READERS_ISO7816_API SAMAV2ISO7816Commands
    : public SAMISO7816Commands<KeyEntryAV2Information, SETAV2>
#ifndef SWIG
      ,
      public SAMAV2Commands<KeyEntryAV2Information, SETAV2>
#endif
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV2ISO7816Commands();

    explicit SAMAV2ISO7816Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~SAMAV2ISO7816Commands();

    void authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno) override;

    void authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno,
                          sam::HostMode hostmode);

    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>>
    getKeyEntry(unsigned char keyno) override;
    std::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno) override;

    void changeKUCEntry(unsigned char kucno, std::shared_ptr<SAMKucEntry> kucentry,
                        std::shared_ptr<DESFireKey> key) override;
    void
    changeKeyEntry(unsigned char keyno,
                   std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2>> keyentry,
                   std::shared_ptr<DESFireKey> key) override;

    ByteVector transmit(ByteVector cmd, bool first = true, bool last = true, bool s_mode = false) override;

    ByteVector dumpSecretKey(unsigned char keyno, unsigned char keyversion,
                             ByteVector divInput) override;

    void activateOfflineKey(unsigned char keyno, unsigned char keyversion,
                            ByteVector divInput) override;

    ByteVector decipherOfflineData(ByteVector data) override;

    ByteVector encipherOfflineData(ByteVector data) override;
	
	void changeKeyEntryOffline(unsigned char keyno, const KeyEntryUpdateSettings& updateSettings, unsigned short changecnt, const ByteVector& encke) override;
	
	void changeKUCEntryOffline(unsigned char kucno, const KucEntryUpdateSettings& updateSettings, unsigned short changecnt, const ByteVector& enckuc) override;
	
	void disableKeyEntryOffline(unsigned char keyno, unsigned short changecnt, const ByteVector& encuid) override;

    virtual ByteVector cmacOffline(const ByteVector &data);

    std::shared_ptr<Chip> getChip() const override
    {
        return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getChip();
    }

    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getReaderCardAdapter();
    }

    void generateOfflineSessionKey(std::shared_ptr<DESFireKey> key, unsigned short changecnt);

    
    void PKI_GenerateKeyPair(unsigned char keyNo, unsigned short configSettings,
                             unsigned char keyNoCEK, unsigned char keyNoVCEK,
                             unsigned char keyNoRef, unsigned char *keyNoAEK = nullptr,
                             unsigned char *keyVAEK = nullptr, unsigned short nLen = 0x40,
                             const ByteVector &pki_e = {},
                             bool includeAccess      = false) override;

    void PKI_ImportKey(unsigned char keyNo, unsigned short configSettings,
                       unsigned char keyNoCEK, unsigned char keyNoVCEK,
                       unsigned char refNoKUC, const ByteVector &pki_n,
                       const ByteVector &pki_e, const ByteVector &pki_p = {},
                       const ByteVector &pki_q = {}, const ByteVector &pki_dP = {},
                       const ByteVector &pki_dQ = {}, const ByteVector &pki_ipq = {},
                       unsigned char *keyNoAEK = nullptr,
                       unsigned char *keyVAEK = nullptr, bool includeAccess = false,
                       bool updateSettingsOnly = false) override;

    ByteVector PKI_ExportPrivateKey(unsigned char keyNo, bool returnAEK = false) override;

    ByteVector PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK = false) override;

    ByteVector
    PKI_UpdateKeyEntries(EVP_PKEY *encKey, EVP_PKEY *signKey, unsigned char keyNoEnc,
                         unsigned char keyNoSign, unsigned char keyNoAck,
                         unsigned char hashAlgo,
                         const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
                         uint16_t changeCounter) override;

    ByteVector PKI_UpdateKeyEntries(unsigned char keyNoEnc, unsigned char keyNoSign,
                                    unsigned char keyNoAck, unsigned char hashAlgo,
                                    unsigned char nbKeyEntries,
                                    const ByteVector &encKeyFrame,
                                    const ByteVector &signature) override;


    ByteVector PKI_EncipherKeyEntries(
        unsigned char hashAlgo, unsigned char keyNoEnc, unsigned char keyNoSign,
        unsigned char keyNoDec, unsigned char keyNoVerif, unsigned short persoCtr,
        const std::vector<std::pair<unsigned char, unsigned char>> &keyEntries,
        const ByteVector &divInput = {}) override;

    ByteVector PKI_GenerateHash(unsigned char hashAlgo,
                                const ByteVector &message) override;

    void PKI_GenerateSignature(unsigned char hashAlgo, unsigned char keyNoSign,
                               const ByteVector &hash) override;

    ByteVector PKI_SendSignature() override;

    void PKI_VerifySignature(unsigned char hashAlgo, unsigned char keyNoVerif,
                             const ByteVector &hash,
                             const ByteVector &signature) override;

    ByteVector PKI_EncipherData(unsigned char hashAlgo, unsigned char keyNoEnc,
                                const ByteVector &plainData) override;

    ByteVector PKI_DecipherData(unsigned char hashAlgo, unsigned char keyNoDec,
                                const ByteVector &encData) override;

    void PKI_ImportEccKey(unsigned char keyNo, unsigned short eccSet,
                          unsigned char keyNoCEK, unsigned char keyNoVCEK,
                          unsigned char keyNoKUC, unsigned char keyNoAEK,
                          unsigned char keyNoVAEK,
                          const ByteVector &eccPublicKey = ByteVector(),
                          bool settingsOnly              = false) override;

    void PKI_ImportEccCurve(unsigned char curveNo, unsigned char keyNoCCK,
                            unsigned char keyNoVCCK,
                            const ByteVector &eccCurve = ByteVector(),
                            bool settingsOnly          = false) override;

    ByteVector PKI_ExportEccPublicKey(unsigned char keyNo) override;

    void PKI_VerifyEccSignature(unsigned char keyNo, unsigned char curveNo,
                                const ByteVector &message,
                                const ByteVector &signature) override;


  protected:
    void generateSessionKey(ByteVector rnd1, ByteVector rnd2);

    ByteVector createfullProtectionCmd(const ByteVector &cmd);
    ByteVector createMacProtectionCmd(const ByteVector &cmd);

    ByteVector transmitSecureChained(const ByteVector &cmd, unsigned char PxIndex = 3, bool hasLe = false);
    
    std::pair<ByteVector, ByteVector> prepareSecureChainedPayload(const ByteVector &payload, bool le = false);
    std::vector<ByteVector> createSecureChainedApduFrames(const ByteVector &cmd,
                                                          const ByteVector &encData,
                                                          const ByteVector &mac,
                                                          unsigned char PxIndex,
                                                          bool le = false);

    ByteVector verifyAndDecryptResponse(const ByteVector &response);
    ByteVector verifyAndDecryptMacResponse(const ByteVector &response);

    static void getLcLe(const ByteVector &cmd, bool &lc, unsigned char &lcvalue);

    ByteVector generateEncIV(bool encrypt) const;

    ByteVector buildPlaintext(uint16_t changeCtr, const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries);
    ByteVector rsa_oaep_encrypt(EVP_PKEY *pubKey, const ByteVector &plaintext, const EVP_MD *md);
    ByteVector rsa_pss_sign(EVP_PKEY *privKey, const ByteVector &data, const EVP_MD *md);
    const EVP_MD *getHash(uint8_t hashAlgo);
    void buildCryptogram(EVP_PKEY *encKey, EVP_PKEY *signKey, uint8_t keyNoEnc, uint8_t keyNoSign, uint16_t changeCtr,
        const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
        uint8_t hashAlgo, ByteVector &encFrame, ByteVector &signature);

    void resetIVs();
    void secureZero(ByteVector &vec);

    ByteVector d_macSessionKey;

    ByteVector d_lastMacIV;

    unsigned int d_cmdCtr;
};
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */