/**
 * \file mifareok5553commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Rpleth Commands
 */

#ifndef LOGICALACCESS_MIFAREOK5553COMMANDS_HPP
#define LOGICALACCESS_MIFAREOK5553COMMANDS_HPP

#include "mifare/mifarecommands.hpp"
#include "../readercardadapters/ok5553readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFAREOK5553 "MifareOK5553"

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

        explicit MifareOK5553Commands(std::string);

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
        ByteVector readBinary(unsigned char blockno, size_t len) override;

        /**
         * \brief Write bytes to the card.
         * \param blockno The block number.
         * \param buf The buffer containing the data.
         * \param buflen The length of buffer.
         * \return The count of bytes written.
         */
        void updateBinary(unsigned char blockno, const ByteVector& buf) override;

        /**
         * \brief Load a key to the reader.
         * \param keyno The reader key slot number. Can be anything from 0x00 to 0x1F.
         * \param keytype The key type.
         * \param key The key.
         * \param vol Use volatile memory, not used by this reader.
         * \return true on success, false otherwise.
         */
        bool loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol = false) override;

        /**
         * \brief Load a key on a given location.
         * \param location The location.
		 * \param keytype The mifare key type.
         * \param key The key.
         */
		void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype, std::shared_ptr<MifareKey> key) override;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype) override;

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param key_storage The key storage used for authentication.
         * \param keytype The key type.
         * \return true if authenticated, false otherwise.
         */
        void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype) override;

		/**
		* \brief Increment a block value.
		* \param blockno The block number.
		* \param value The increment value.
		*/
	    void increment(unsigned char blockno, unsigned int value) override;

		/**
		* \brief Decrement a block value.
		* \param blockno The block number.
		* \param value The decrement value.
		*/
	    void decrement(unsigned char blockno, unsigned int value) override;

        /**
         * \brief Get the OK5553 reader/card adapter.
         * \return The OK5553 reader/card adapter.
         */
        virtual std::shared_ptr<OK5553ReaderCardAdapter> getOK5553ReaderCardAdapter() { return std::dynamic_pointer_cast<OK5553ReaderCardAdapter>(getReaderCardAdapter()); }
    };
}

#endif /* LOGICALACCESS_MIFAREOK5553COMMANDS_HPP */