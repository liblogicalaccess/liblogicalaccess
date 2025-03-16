#pragma once

#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>

namespace logicalaccess
{
#ifndef DFEV3_INS
#define DFEV3_INS
#define DFEV3_INS_GET_FILE_COUNTERS 0xF6
#endif
class DESFireEV3Chip;

/**
 * \brief The DESFire EV3 base commands class.
 *
 * We do not reimplement or add anything yet.
 */
class LLA_CARDS_DESFIRE_API DESFireEV3Commands : public DESFireEV2Commands
{
  public:
    // We inherit everything from EV2.
    // We are backward compat mode only.
    ~DESFireEV3Commands() override;

    /**
     * \brief Get file related counters used for SDM.
     * \param fileno The file number.
     * \return The file counters.
     */
    virtual ByteVector getFileCounters(unsigned char fileno) = 0;

    virtual void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                                   const DESFireAccessRights &accessRights,
                                   unsigned int fileSize,
                                   unsigned short isoFID,
                                   bool multiAccessRights,
                                   bool sdmAndMirroring) = 0;

    virtual void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
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
                                    unsigned int sdmReadCtrLimitValue) = 0;

    virtual bool performAESOriginalityCheck() = 0;

  private:
    std::shared_ptr<DESFireEV3Chip> getDESFireEV3Chip() const;
};

}
