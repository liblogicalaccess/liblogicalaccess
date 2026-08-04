/**
 * \file SAMAV3Commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV3COMMANDS_HPP
#define LOGICALACCESS_SAMAV3COMMANDS_HPP

#include <logicalaccess/plugins/cards/samav/samcommands.hpp>

namespace logicalaccess
{
template <typename T, typename S>
class SAMAV3Commands : public ICommands
{
  public:
    virtual ByteVector encipherKeyEntry(unsigned char keyno,
                                  unsigned char targetKeyno,
                                  unsigned short changeCounter,
                                  unsigned char channel = 0,
                                  const ByteVector& targetSamUid = ByteVector(),
                                  const ByteVector& divInput = ByteVector()) = 0;

    virtual void PKI_ImportCaPk(const ByteVector &rid, unsigned char pkId,
                                      unsigned short set, unsigned char keyNoCEK,
                                      unsigned char keyVCEK, unsigned char keyNoAEK,
                                      unsigned char keyVAEK, unsigned char pkExpLen,
                                      const ByteVector &pk       = ByteVector(),
                                      const ByteVector &checkSum = ByteVector(),
                                      bool settingsOnly          = false) = 0;

    virtual ByteVector PKI_ImportCaPkOffline(const ByteVector &offlineCryptogram,
                                     bool settingsOnly, bool expectResponseData) = 0;

    virtual void PKI_RemoveCaPk(const ByteVector &rid, unsigned char pkId) = 0;

    virtual ByteVector PKI_RemoveCaPkOffline(unsigned short changeCounter, const ByteVector &encCaPkRef,
                                             const ByteVector &offlineMac, bool expectResponseData) = 0;

    virtual ByteVector PKI_ExportCaPk(const ByteVector &rid, unsigned char pkId, bool settingsOnly) = 0;

    virtual ByteVector PKI_LoadIssuerPk(const ByteVector &rid, unsigned char pkId,
                                        const ByteVector &issuerPkCert,
                                        const ByteVector &issuerPkRemainder,
                                        unsigned char pkExpLen) = 0;

    virtual ByteVector PKI_LoadIccPk(const ByteVector &iccPkCert,
                                     const ByteVector &iccPkRemainder,
                                     const ByteVector &staticData, unsigned char pkExpLen) = 0;

    virtual ByteVector SAM_RecoverStaticData(const ByteVector &ssad) = 0;

    virtual ByteVector SAM_RecoverDynamicData(const ByteVector &sdad) = 0;

    virtual ByteVector SAM_EncipherPIN(const ByteVector &pinBlock, const ByteVector &iccNumber) = 0;
};
}

#endif /* LOGICALACCESS_SAMAV3COMMANDS_HPP */