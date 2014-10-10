/**
 * \file mifareplussl1commands.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP

#include "../mifare/mifarecommands.hpp"
#include "mifarepluscommands.hpp"
#include "mifareplusaccessinfo.hpp"

namespace logicalaccess
{
    class MifarePlusChip;

    /**
     * \brief The MifarePlus SL1 commands class.
     */
    class LIBLOGICALACCESS_API MifarePlusSL1Commands : public virtual MifareCommands, public virtual MifarePlusCommands
    {
    public:

        /**
         * \brief Read a whole sector.
         * \param sector The sector number, from 0 to 15.
         * \param start_block The first block on the sector.
         * \param buf A buffer to fill with the data of the sector.
         * \param buflen The length of buffer. Must be at least 48 bytes long or the call will fail.
         * \param sab The sector access bits.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual size_t readSector(int sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab);

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
        virtual size_t writeSector(int sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte = 0x00, MifarePlusAccessInfo::SectorAccessBits* newsab = NULL);

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
        virtual size_t readSectors(int start_sector, int stop_sector, int start_block, void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab);

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
        virtual size_t writeSectors(int start_sector, int stop_sector, int start_block, const void* buf, size_t buflen, const MifarePlusAccessInfo::SectorAccessBits& sab, unsigned char userbyte = 0x00, MifarePlusAccessInfo::SectorAccessBits* newsab = NULL);

        /**
         * \brief Get the sector referenced by the AID from the MAD.
         * \param aid The application ID.
         * \param madKeyA The MAD key A for read access.
         * \return The sector.
         */
        unsigned int getSectorFromMAD(long aid, boost::shared_ptr<MifarePlusKey> madKeyA);

        /**
         * \brief Set the sector referenced by the AID to the MAD.
         * \param aid The application ID.
         * \param sector The sector.
         * \param madKeyA The MAD key A for read access.
         * \param madKeyB The MAD key B for write access.
         */
        void setSectorToMAD(long aid, unsigned int sector, boost::shared_ptr<MifarePlusKey> madKeyA, boost::shared_ptr<MifarePlusKey> madKeyB);

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
        bool authenticate(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> ai);

        void changeKey(boost::shared_ptr<MifarePlusKey> keyA, boost::shared_ptr<MifarePlusKey> keyB, unsigned int sector, const MifarePlusAccessInfo::SectorAccessBits& sab, MifarePlusAccessInfo::SectorAccessBits* newsab, unsigned char userbyte = 0x00);

        /**
         * \brief Change authenticated block.
         * \param sab The security access bits.
         * \param sector The sector.
         * \param block The block.
         * \param write Write access.
         */
        void changeBlock(const MifarePlusAccessInfo::SectorAccessBits& sab, int sector, int block, bool write);

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

        /**
         * \brief Load a key on a given location.
         * \param location The location.
         * \param key The key.
         * \param keytype The mifare plus key type.
         */
        virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifarePlusKeyType keytype) = 0;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param key_storage The key storage used for authentication.
         * \param keytype The mifare plus key type.
         * \return true if authenticated, false otherwise.
         */
        virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifarePlusKeyType keytype) = 0;

        /**
        * \brief Switch to SL2
        * \param key The switch SL2 AES key
        */
        virtual bool SwitchLevel2(boost::shared_ptr<MifarePlusKey> key) = 0;

        virtual bool AESAuthenticate(boost::shared_ptr<MifarePlusKey> key) = 0;

    protected:

        boost::shared_ptr<MifarePlusChip> getMifarePlusChip();
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP */