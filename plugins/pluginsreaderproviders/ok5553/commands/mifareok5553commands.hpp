/**
 * \file mifareok5553commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Rpleth Commands
 */

#ifndef LOGICALACCESS_MIFAREOK5553COMMANDS_HPP
#define LOGICALACCESS_MIFAREOK5553COMMANDS_HPP

#include "mifarecommands.hpp"
#include "../readercardadapters/ok5553readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief A Mifare OK5553 commands base class.
     */
    class LIBLOGICALACCESS_API MifareOK5553Commands : public MifareCommands
    {
    public:
        /**
         * \brief Constructor.
         */
        MifareOK5553Commands();

        /**
         * \brief Destructor.
         */
        virtual ~MifareOK5553Commands();

        /**
         * \brief Read bytes from the card.
         * \param blockno The block number.
         * \param len The count of bytes to read. (0 <= len < 16)
         * \param buf The buffer in which to place the data.
         * \param buflen The length of buffer.
         * \return The count of bytes red.
         */
        size_t readBinary(unsigned char blockno, size_t len, void* buf, size_t buflen);

        /**
         * \brief Write bytes to the card.
         * \param blockno The block number.
         * \param buf The buffer containing the data.
         * \param buflen The length of buffer.
         * \return The count of bytes written.
         */
        size_t updateBinary(unsigned char blockno, const void* buf, size_t buflen);

        /**
         * \brief Load a key to the reader.
         * \param keyno The reader key slot number. Can be anything from 0x00 to 0x1F.
         * \param keytype The key type.
         * \param key The key byte array.
         * \param keylen The key byte array length.
         * \param vol Use volatile memory, not used by this reader.
         * \return true on success, false otherwise.
         */
        bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false);

        /**
         * \brief Load a key on a given location.
         * \param location The location.
         * \param key The key.
         * \param keytype The mifare key type.
         */
        void loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype);

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param key_storage The key storage used for authentication.
         * \param keytype The key type.
         * \return true if authenticated, false otherwise.
         */
        void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype);

        /**
         * \brief Get the OK5553 reader/card adapter.
         * \return The OK5553 reader/card adapter.
         */
        virtual std::shared_ptr<OK5553ReaderCardAdapter> getOK5553ReaderCardAdapter() { return std::dynamic_pointer_cast<OK5553ReaderCardAdapter>(getReaderCardAdapter()); };
    };
}

#endif /* LOGICALACCESS_MIFAREOK5553COMMANDS_HPP */