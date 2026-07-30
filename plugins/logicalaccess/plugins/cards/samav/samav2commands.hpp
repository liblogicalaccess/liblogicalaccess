/**
 * \file SAMAV2Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2COMMANDS_HPP
#define LOGICALACCESS_SAMAV2COMMANDS_HPP

#include <logicalaccess/plugins/cards/samav/samtypes.hpp>
#include <logicalaccess/plugins/cards/samav/samcommands.hpp>
#include <openssl/rsa.h>

namespace logicalaccess
{

// todo: We removed export macro to fix link issue. need to investigate more.
template <typename T, typename S>
class SAMAV2Commands : public ICommands
{
  public:
    virtual ByteVector dumpSecretKey(unsigned char keyno, unsigned char keyversion, const ByteVector& divInput) = 0;

    virtual void activateOfflineKey(unsigned char keyno, unsigned char keyversion, const ByteVector& divInput) = 0;

    virtual ByteVector decipherOfflineData(const ByteVector &data) = 0;

    virtual ByteVector encipherOfflineData(const ByteVector &data) = 0;
	
    virtual void changeKeyEntryOffline(unsigned char keyno, const KeyEntryUpdateSettings& updateSettings,
        unsigned short changecnt, const ByteVector& encke) = 0;
	
    virtual void changeKUCEntryOffline(unsigned char kucno, const KucEntryUpdateSettings& updateSettings,
        unsigned short changecnt, const ByteVector& enckuc) = 0;
	
    virtual void disableKeyEntryOffline(unsigned char keyno, unsigned short changecnt, const ByteVector& encuid) = 0;

    virtual void PKI_GenerateKeyPair(unsigned char keyNo, unsigned short configSettings, unsigned char keyNoCEK,
            unsigned char keyNoVCEK, unsigned char keyNoRef, const sam::AEKVAEK &accessKeys,
            unsigned short nLen = 0x40, unsigned short eLen = 0x04, const ByteVector &pki_e = {},
            bool includeAccess = false) = 0;

    virtual void PKI_ImportKey(unsigned char keyNo, unsigned short configSettings,
                      unsigned char keyNoCEK, unsigned char keyNoVCEK,
                      unsigned char refNoKUC, const ByteVector &pki_n,
                      const ByteVector &pki_e, const ByteVector &pki_p = {},
                      const ByteVector &pki_q = {}, const ByteVector &pki_dP = {},
                      const ByteVector &pki_dQ = {}, const ByteVector &pki_ipq = {},
                      const sam::AEKVAEK &accessKeys = {}, bool includeAccess = false,
                      bool updateSettingsOnly = false) = 0;

    virtual ByteVector PKI_ExportPrivateKey(unsigned char keyNo, bool returnAEK = false) = 0;

    virtual ByteVector PKI_ExportPublicKey(unsigned char keyNo, bool returnAEK = false) = 0;

    virtual ByteVector PKI_UpdateKeyEntries(const ByteVector &encPublicKeyDer, const ByteVector &signPrivateKeyDer,
                         unsigned char keyNoEnc, unsigned char keyNoSign, bool requestAck,
                         unsigned char keyNoAck, unsigned char hashAlgo,
                         const std::vector<std::shared_ptr<SAMBasicKeyEntry>> &entries,
                         uint16_t changeCounter) = 0;

    virtual ByteVector PKI_UpdateKeyEntries(unsigned char keyNoEnc, unsigned char keyNoSign,
                             bool requestAck, unsigned char keyNoAck,
                             unsigned char hashAlgo, unsigned char nbKeyEntries,
                             const ByteVector &encKeyFrame, const ByteVector &signature) = 0;

    virtual ByteVector PKI_EncipherKeyEntries(unsigned char hashAlgo, unsigned char keyNoEnc,
            unsigned char keyNoSign, unsigned char keyNoDec, unsigned char keyNoVerif,
            unsigned short persoCtr, const std::vector<std::pair<unsigned char, unsigned char>> &keyEntries,
            const ByteVector &divInput = {}) = 0;

    virtual ByteVector PKI_GenerateHash(unsigned char hashAlgo, const ByteVector &message) = 0;

    virtual void PKI_GenerateSignature(unsigned char hashAlgo, unsigned char keyNoSign, const ByteVector &hash) = 0;

    virtual ByteVector PKI_SendSignature() = 0;

    virtual void PKI_VerifySignature(unsigned char hashAlgo, unsigned char keyNoVerif,
                                         const ByteVector &hash,
                                         const ByteVector &signature) = 0;

    virtual ByteVector PKI_EncipherData(unsigned char hashAlgo,
                                            unsigned char keyNoEnc,
                                            const ByteVector &plainData) = 0;

    virtual ByteVector PKI_DecipherData(unsigned char hashAlgo,
                                            unsigned char keyNoDec,
                                            const ByteVector &encData) = 0;

    virtual void PKI_ImportEccKey(unsigned char keyNo, unsigned short eccSet,
                                      unsigned char keyNoCEK, unsigned char keyNoVCEK,
                                      unsigned char keyNoKUC, unsigned char keyNoAEK,
                                      unsigned char keyNoVAEK,
                                      const ByteVector &eccPublicKey = ByteVector(),
                                      bool settingsOnly              = false) = 0;

    virtual void PKI_ImportEccCurve(unsigned char curveNo, unsigned char keyNoCCK,
                                        unsigned char keyNoVCCK,
                                        const ByteVector &eccCurve = ByteVector(),
                                        bool settingsOnly          = false) = 0;

    virtual ByteVector PKI_ExportEccPublicKey(unsigned char keyNo) = 0;

    virtual void PKI_VerifyEccSignature(unsigned char keyNo, unsigned char curveNo,
                                            const ByteVector &message,
                                            const ByteVector &signature) = 0;
};
}

#endif /* LOGICALACCESS_SAMAV2COMMANDS_HPP */