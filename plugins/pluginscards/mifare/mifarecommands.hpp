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
    class MifareChip;

    /**
     * \brief The Mifare commands class.
     */
    class LIBLOGICALACCESS_API MifareCommands : public virtual Commands
    {
    public:

        /**
         * \brief Read a whole sector.
         * \param sector The sector number, from 0 to 15.
         * \param start_block The first block on the sector.
         * \param buf A buffer to fill with the data of the sector.
         * \param buflen The length of buffer. Must be at least 48 bytes long or the call will fail.
         * \param sab The sector access bits.
         * \param readtrailer Read the trailer block.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual std::vector<unsigned char> readSector(int sector, int start_block,
                                                      const MifareAccessInfo::SectorAccessBits& sab,
                                                      bool readtrailer = false) final;

        /**
         * \brief Write a whole sector.
         * \param sector The sector number, from 0 to 15.
         * \param start_block The first block on the sector.
         * \param buf A buffer to from which to copy the data.
         * \param buflen The length of buffer. Must be at least 48 bytes long or the call will fail.
         * \param sab The sector access bits.
         * \param userbyte The user byte on trailer block.
         * \param newsab If not NULL the key will be changed as well.
         * \return The number of bytes written, or a negative value on error.
         * \warning If sector is 0, the first 16 bytes will be skipped and be considered "copied" since the first block in sector 0 is read-only.
         */
        virtual void writeSector(int sector, int start_block,
                                 const std::vector<unsigned char>& buf,
                                 const MifareAccessInfo::SectorAccessBits& sab,
                                 unsigned char userbyte = 0x00,
                                 MifareAccessInfo::SectorAccessBits* newsab = NULL) final;

        /**
         * \brief Read several sectors.
         * \param start_sector The start sector number, from 0 to stop_sector.
         * \param stop_sector The stop sector number, from start_sector to 15.
         * \param start_block The first block on the sector.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_sector - start_sector + 1) * 48 bytes long.
         * \param sab The sector access bits.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual std::vector<unsigned char> readSectors(int start_sector, int stop_sector,
                                                       int start_block,
                                                       const MifareAccessInfo::SectorAccessBits& sab) final;

        /**
         * \brief Write several sectors.
         * \param start_sector The start sector number, from 0 to stop_sector.
         * \param stop_sector The stop sector number, from start_sector to 15.
         * \param start_block The first block on the sector.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_sector - start_sector + 1) * 48 bytes long.
         * \param sab The sector access bits.
         * \param userbyte The user byte on trailer blocks.
         * \param newsab If not NULL the keys will be changed as well.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual void writeSectors(int start_sector, int stop_sector,
                                  int start_block, const std::vector<unsigned char>& buf,
                                  const MifareAccessInfo::SectorAccessBits& sab,
                                  unsigned char userbyte = 0x00,
                                  MifareAccessInfo::SectorAccessBits* newsab = NULL) final;

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
        void setSectorToMAD(long aid, unsigned int sector, std::shared_ptr<MifareKey> madKeyA, std::shared_ptr<MifareKey> madKeyB);

        /**
         * \brief Calculate the MAD crc.
         * \param madbuf The MAD buffer.
         * \param madbuflen The MAD buffer length.
         * \return The CRC.
         */
        static unsigned char calculateMADCrc(const unsigned char* madbuf, size_t madbuflen);

        /**
         * \brief Find the referenced sector in the MAD.
         * \param aid The application ID.
         * \param madbuf The MAD buffer.
         * \param madbuflen The MAD buffer length.
         */
        static unsigned int findReferencedSector(long aid, unsigned char* madbuf, size_t madbuflen);

        /**
         * \brief Authenticate on a given location.
         * \param location The location.
         * \param ai The access infos.
         */
        void authenticate(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> ai);

        /**
         * \brief Load a key on a given location.
         * \param location The location.
         * \param key The key.
         * \param keytype The mifare key type.
         */
        //virtual void loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype);

        void changeKey(std::shared_ptr<MifareKey> keyA, std::shared_ptr<MifareKey> keyB, unsigned int sector, const MifareAccessInfo::SectorAccessBits& sab, MifareAccessInfo::SectorAccessBits* newsab, unsigned char userbyte = 0x00);

        /**
         * \brief Change authenticated block.
         * \param sab The security access bits.
         * \param sector The sector.
         * \param block The block.
         * \param write Write access.
         */
        void changeBlock(const MifareAccessInfo::SectorAccessBits& sab, int sector, int block, bool write);

        /**
         * \brief Read bytes from the card.
         * \param blockno The block number.
         * \param len The count of bytes to read. (0 <= len < 16)
         * \return The count of bytes red.
         */
        virtual std::vector<unsigned char> readBinary(unsigned char blockno, size_t len) = 0;

        /**
         * \brief Write bytes to the card.
         * \param blockno The block number.
         * \param buf The buffer containing the data.
         * \param buflen The length of buffer.
         * \return The count of bytes written.
         */
        virtual void updateBinary(unsigned char blockno, const std::vector<unsigned char>& buf) = 0;

        /**
         * \brief Load a key to the reader.
         * \param keyno The reader key slot number. Can be anything from 0x00 to 0x1F.
         * \param keytype The key type.
         * \param key The key byte array.
         * \param keylen The key byte array length.
         * \param vol Use volatile memory.
         * \return true on success, false otherwise.
         */
        virtual bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false) = 0;

        /**
         * \brief Load a key on a given location.
         * \param location The location.
         * \param key The key.
         * \param keytype The mifare key type.
         */
        virtual void loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype) = 0;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        virtual void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype) = 0;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param key_storage The key storage used for authentication.
         * \param keytype The key type.
         */
        virtual void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype) = 0;

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