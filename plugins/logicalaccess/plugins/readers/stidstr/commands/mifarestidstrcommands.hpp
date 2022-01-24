/**
 * \file mifarestidstrcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare STidSTR card.
 */

#ifndef LOGICALACCESS_MIFARESTIDSTRCOMMANDS_HPP
#define LOGICALACCESS_MIFARESTIDSTRCOMMANDS_HPP

#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/readers/stidstr/readercardadapters/stidstrreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFARESTIDSTR "MifareSTidSTR"

/**
 * \brief The Mifare commands class for STidSTR reader.
 */
class LLA_READERS_STIDSTR_API MifareSTidSTRCommands : public MifareCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareSTidSTRCommands();

    explicit MifareSTidSTRCommands(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~MifareSTidSTRCommands();

    /**
     * \brief Get the STidSTR reader/card adapter.
     * \return The STidSTR reader/card adapter.
     */
    std::shared_ptr<STidSTRReaderCardAdapter> getSTidSTRReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<STidSTRReaderCardAdapter>(
            getReaderCardAdapter());
    }

    /**
     * \brief Scan the RFID field for Mifare tag.
     * \return The tag uid if present.
     */
    ByteVector scanMifare() const;

    /**
     * \brief Release the RFID field.
     */
    void releaseRFIDField() const;

    /**
     * \brief Load a key on a given location.
     * \param location The location.
     * \param key The key.
     * \param keytype The mifare key type.
     */
    void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype,
                 std::shared_ptr<MifareKey> key) override;

    /**
     * \brief Load a key to the reader.
     * \param keyno The key number.
     * \param keytype The key type.
     * \param key The key.
     * \param vol Use volatile memory.
     * \return true on success, false otherwise.
     */
    bool loadKey(unsigned char keyno, MifareKeyType keytype,
                 std::shared_ptr<MifareKey> key, bool vol = true) override;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param keyno The key number, previously loaded with Mifare::loadKey().
     * \param keytype The key type.
     */
    void authenticate(unsigned char blockno, unsigned char keyno,
                      MifareKeyType keytype) override;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param key_storage The key storage used for authentication.
     * \param keytype The key type.
     */
    void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage,
                      MifareKeyType keytype) override;

    /**
     * \brief Read bytes from the card.
     * \param blockno The block number.
     * \param len The count of bytes to read. (0 <= len < 16)
     * \param buf The buffer in which to place the data.
     * \param buflen The length of buffer.
     * \return The count of bytes red.
     */
    ByteVector readBinary(unsigned char blockno, size_t len) override;

    /**
     * \brief Write bytes to the card.
     * \param blockno The block number.
     * \param buf The buffer containing the data.
     * \param buflen The length of buffer.
     * \return The count of bytes written.
     */
    void updateBinary(unsigned char blockno, const ByteVector &buf) override;

    /**
    * \brief Increment a block value.
    * \param blockno The block number.
    * \param value The increment value.
    */
    void increment(unsigned char blockno, uint32_t value) override;

    /**
    * \brief Decrement a block value.
    * \param blockno The block number.
    * \param value The decrement value.
    */
    void decrement(unsigned char blockno, uint32_t value) override;

  protected:
    /**
     * \brief Read bytes from the card.
     * \param keyindex The key index.
     * \param blockno The block number.
     * \param len The count of bytes to read. (0 <= len < 16)
     * \param buf The buffer in which to place the data.
     * \param buflen The length of buffer.
     * \return The count of bytes red.
     */
    virtual ByteVector readBinaryIndex(unsigned char keyindex, unsigned char blockno,
                                       size_t len);

    /**
     * \brief Write bytes to the card.
     * \param keyindex The key index.
     * \param blockno The block number.
     * \param buf The buffer containing the data.
     * \param buflen The length of buffer.
     * \return The count of bytes written.
     */
    virtual void updateBinaryIndex(unsigned char keyindex, unsigned char blockno,
                                   const ByteVector &buf);

    /**
     * \brief Use SKB for read/write operation, or not.
     */
    bool d_useSKB;

    /**
     * \brief The SKB key index if SKB use.
     */
    unsigned char d_skbIndex;

    /**
     * \brief The last key type used;
     */
    MifareKeyType d_lastKeyType;
};
}

#endif /* LOGICALACCESS_MIFARESTIDSTRCOMMANDS_H */