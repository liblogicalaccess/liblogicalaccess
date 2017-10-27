/**
 * \file mifarecommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare commands.
 */

#ifndef LOGICALACCESS_MIFARECOMMANDS_HPP
#define LOGICALACCESS_MIFARECOMMANDS_HPP

#include "mifareaccessinfo.hpp"
#include "logicalaccess/cards/commands.hpp"
#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
#define CMD_MIFARE "Mifare"

class MifareChip;

/**
 * \brief The Mifare commands class.
 */
class LIBLOGICALACCESS_API MifareCommands : public Commands
{
  public:
    MifareCommands()
        : Commands(CMD_MIFARE)
    {
    }

    explicit MifareCommands(std::string ct)
        : Commands(ct)
    {
    }

    virtual ByteVector readSector(int sector, int start_block,
                                  std::shared_ptr<MifareKey> keyA,
                                  std::shared_ptr<MifareKey> keyB,
                                  const MifareAccessInfo::SectorAccessBits &sab,
                                  bool readtrailer = false) final;

    virtual void
    writeSector(int sector, int start_block, const ByteVector &buf,
                std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB,
                const MifareAccessInfo::SectorAccessBits &sab,
                unsigned char userbyte                     = 0x00,
                MifareAccessInfo::SectorAccessBits *newsab = nullptr,
                std::shared_ptr<MifareKey> newkeyA         = std::shared_ptr<MifareKey>(),
                std::shared_ptr<MifareKey> newkeyB = std::shared_ptr<MifareKey>()) final;

    virtual ByteVector readSectors(int start_sector, int stop_sector, int start_block,
                                   std::shared_ptr<MifareKey> keyA,
                                   std::shared_ptr<MifareKey> keyB,
                                   const MifareAccessInfo::SectorAccessBits &sab) final;

    virtual void writeSectors(
        int start_sector, int stop_sector, int start_block, const ByteVector &buf,
        std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB,
        const MifareAccessInfo::SectorAccessBits &sab, unsigned char userbyte = 0x00,
        MifareAccessInfo::SectorAccessBits *newsab = nullptr,
        std::shared_ptr<MifareKey> newkeyA         = std::shared_ptr<MifareKey>(),
        std::shared_ptr<MifareKey> newkeyB         = std::shared_ptr<MifareKey>()) final;

    /**
     * \brief Get the sector referenced by the AID from the MAD.
     * \param aid The application ID.
     * \param madKeyA The MAD key A for read access.
     * \return The sector.
     */
    unsigned int getSectorFromMAD(long aid, std::shared_ptr<MifareKey> madKeyA);

    /**
     * \brief Set the sector referenced by the AID to the MAD.
     * \param aid The application ID.
     * \param sector The sector.
     * \param madKeyA The MAD key A for read access.
     * \param madKeyB The MAD key B for write access.
     */
    void setSectorToMAD(long aid, unsigned int sector, std::shared_ptr<MifareKey> madKeyA,
                        std::shared_ptr<MifareKey> madKeyB);

    /**
     * \brief Calculate the MAD crc.
     * \param madbuf The MAD buffer.
     * \param madbuflen The MAD buffer length.
     * \return The CRC.
     */
    static unsigned char calculateMADCrc(const unsigned char *madbuf, size_t madbuflen);

    /**
     * \brief Find the referenced sector in the MAD.
     * \param aid The application ID.
     * \param madbuf The MAD buffer.
     * \param madbuflen The MAD buffer length.
     */
    static unsigned int findReferencedSector(long aid, unsigned char *madbuf,
                                             size_t madbuflen);

    /**
     * \brief Authenticate on a given location.
     * \param location The location.
     * \param ai The access infos.
     */
    void authenticate(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> ai,
                      bool write = false);

    void changeKeys(MifareKeyType keytype, std::shared_ptr<MifareKey> key,
                    std::shared_ptr<MifareKey> newkeyA,
                    std::shared_ptr<MifareKey> newkeyB, unsigned int sector,
                    MifareAccessInfo::SectorAccessBits *newsab,
                    unsigned char userbyte = 0x00);

    void changeKeys(std::shared_ptr<MifareKey> newkeyA,
                    std::shared_ptr<MifareKey> newkeyB, unsigned int sector,
                    MifareAccessInfo::SectorAccessBits *newsab,
                    unsigned char userbyte = 0x00);

    static MifareKeyType getKeyType(const MifareAccessInfo::SectorAccessBits &sab,
                                    int sector, int block, bool write);

    /**
     * \brief Authenticate for a targeted block.
     * \param keyType The key type.
             * \param key The authentication key.
     * \param sector The sector.
     * \param block The block.
     * \param write Write access.
     */
    void authenticate(MifareKeyType keyType, std::shared_ptr<MifareKey> key, int sector,
                      int block, bool write);

    /**
     * \brief Read bytes from the card.
     * \param blockno The block number.
     * \param len The count of bytes to read. (0 <= len < 16)
     * \return The count of bytes red.
     */
    virtual ByteVector readBinary(unsigned char blockno, size_t len) = 0;

    /**
     * \brief Write bytes to the card.
     * \param blockno The block number.
     * \param buf The buffer containing the data.
     * \param buflen The length of buffer.
     * \return The count of bytes written.
     */
    virtual void updateBinary(unsigned char blockno, const ByteVector &buf) = 0;

    /**
     * \brief Load a key to the reader.
     * \param keyno The reader key slot number. Can be anything from 0x00 to 0x1F.
     * \param keytype The key type.
     * \param key The key.
     * \param vol Use volatile memory.
     * \return true on success, false otherwise.
     */
    virtual bool loadKey(unsigned char keyno, MifareKeyType keytype,
                         std::shared_ptr<MifareKey> key, bool vol = false) = 0;

    /**
     * \brief Load a key on a given location.
     * \param location The location.
             * \param keytype The mifare key type.
     * \param key The key.
     */
    virtual void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype,
                         std::shared_ptr<MifareKey> key) = 0;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param keyno The key number, previously loaded with Mifare::loadKey().
     * \param keytype The key type.
     */
    virtual void authenticate(unsigned char blockno, unsigned char keyno,
                              MifareKeyType keytype) = 0;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param key_storage The key storage used for authentication.
     * \param keytype The key type.
     */
    virtual void authenticate(unsigned char blockno,
                              std::shared_ptr<KeyStorage> key_storage,
                              MifareKeyType keytype) = 0;

    /**
     * Increment (inplace) the value block at address `blockno`.
     *
     * This means that this is the equivalent of also calling transfer.
     *
     * @warning While the parameter is unsigned (because negative value causes errors),
     * value superior to 2^31-1 are also rejected.
     */
    virtual void increment(uint8_t blockno, uint32_t value) = 0;

    /**
     * Decrement (inplace) the value block at address `blockno` by `value` unit.
     *
     * This means that this is the equivalent of also calling transfer.
     *
     * @warning While the parameter is unsigned (because negative value causes errors),
     * value superior to 2^31-1 are also rejected.
     */
    virtual void decrement(uint8_t blockno, uint32_t value) = 0;

    /**
     * Write a value block, using `value` as the numerical value to be written
     * and backup_blockno as the address of the backup block.
     *
     */
    bool writeValueBlock(uint8_t blockno, int32_t value, uint8_t backup_blockno);

    /**
     * Read the value and the address of the backup block from a value-block.
     *
     * If the block is not a valid value block, return false.
     */
    bool readValueBlock(uint8_t blockno, int32_t &value, uint8_t &backup_block);

    /**
     * \brief Get number of data blocks for a sector.
     * \param sector The sector.
     * \return The number of blocks.
     */
    static unsigned char getNbBlocks(int sector);

    /**
     * \brief Get the sector start block.
     * \param sector The sector.
     * \return The first block for this sector.
     */
    static unsigned char getSectorStartBlock(int sector);

  protected:
    std::shared_ptr<MifareChip> getMifareChip() const;
};
}

#endif