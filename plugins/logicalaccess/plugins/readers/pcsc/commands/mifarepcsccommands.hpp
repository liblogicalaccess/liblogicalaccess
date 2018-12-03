/**
 * \file mifarepcsccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare pcsc card.
 */

#ifndef LOGICALACCESS_MIFAREPCSCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREPCSCCOMMANDS_HPP

#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFAREPCSC "MifarePCSC"

template <typename T, typename T2>
class MifarePlusSL1Policy;

/**
 * \brief The Mifare card provider class for PCSC reader.
 */
class LLA_READERS_PCSC_API MifarePCSCCommands : public MifareCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifarePCSCCommands();

    explicit MifarePCSCCommands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifarePCSCCommands();

    /**
     * \brief Get the PC/SC reader/card adapter.
     * \return The PC/SC reader/card adapter.
     */
    std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter());
    }

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

    void increment(uint8_t blockno, uint32_t value) override;

    void decrement(uint8_t blockno, uint32_t value) override;

  protected:
    /**
     * \brief Load a key to the reader.
     * \param keyno The key number.
     * \param keytype The mifare key type.
     * \param key The key.
     * \param vol Use volatile memory.
     * \return true on success, false otherwise.
     */
    bool loadKey(unsigned char keyno, MifareKeyType keytype,
                 std::shared_ptr<MifareKey> key, bool vol = false) override;

    /**
     * \brief Load a key on a given location.
     * \param location The location.
     * \param key The key.
     * \param keytype The mifare key type.
     */
    void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype,
                 std::shared_ptr<MifareKey> key) override;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param key_storage The key storage used for authentication.
     * \param keytype The key type.
     */
    void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage,
                      MifareKeyType keytype) override;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param keyno The key number, previously loaded with Mifare::loadKey().
     * \param keytype The key type.
     */
    void authenticate(unsigned char blockno, unsigned char keyno,
                      MifareKeyType keytype) override;

    template <typename T, typename T2>
    friend class MifarePlusSL1Policy;
};
}

#endif /* LOGICALACCESS_MIFAREPCSCCOMMANDS_HPP */