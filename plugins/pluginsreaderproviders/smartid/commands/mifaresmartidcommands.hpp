/**
 * \file mifaresmartidcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID card.
 */

#ifndef LOGICALACCESS_MIFARESMARTIDCOMMANDS_HPP
#define LOGICALACCESS_MIFARESMARTIDCOMMANDS_HPP

#include "mifarecommands.hpp"
#include "../readercardadapters/mifaresmartidreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare commands class for SmartID reader.
     */
    class LIBLOGICALACCESS_API MifareSmartIDCommands : public MifareCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareSmartIDCommands();

        /**
         * \brief Destructor.
         */
        virtual ~MifareSmartIDCommands();

        /**
         * \brief Get the SmartID reader/card adapter.
         * \return The SmartID reader/card adapter.
         */
        std::shared_ptr<MifareSmartIDReaderCardAdapter> getMifareSmartIDReaderCardAdapter() const { return std::dynamic_pointer_cast<MifareSmartIDReaderCardAdapter>(getReaderCardAdapter()); };

        /**
         * \brief Load a key on a given location.
         * \param location The location.
         * \param key The key.
         * \param keytype The mifare key type.
         */
        virtual void loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifareKeyType keytype);

        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param key_storage The key storage used for authentication.
         * \param keytype The key type.
         */
        virtual void authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifareKeyType keytype);

        /**
         * \brief Load a key to the reader.
         * \param keyno The key number.
         * \param keytype The key type.
         * \param key The key byte array.
         * \param keylen The key byte array length.
         * \param vol Use volatile memory.
         * \return true on success, false otherwise.
         */
        virtual bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false);
        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        virtual void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);

        /**
         * \brief Read bytes from the card.
         * \param blockno The block number.
         * \param len The count of bytes to read. (0 <= len < 16)
         * \param buf The buffer in which to place the data.
         * \param buflen The length of buffer.
         * \return The count of bytes red.
         */
        virtual std::vector<unsigned char> readBinary(unsigned char blockno, size_t len);

        /**
         * \brief Write bytes to the card.
         * \param blockno The block number.
         * \param buf The buffer containing the data.
         * \param buflen The length of buffer.
         * \return The count of bytes written.
         */
        virtual void updateBinary(unsigned char blockno, const std::vector<unsigned char>& buf);

		/**
		* \brief Increment a block value.
		* \param blockno The block number.
		* \param value The increment value.
		*/
		virtual void increment(unsigned char blockno, uint32_t value) override;

		/**
		* \brief Decrement a block value.
		* \param blockno The block number.
		* \param value The decrement value.
		*/
		virtual void decrement(unsigned char blockno, uint32_t value) override;

		/**
		* \brief Transfer volatile memory to block value.
		* \param blockno The block number.
		*/
		virtual void transfer(unsigned char blockno);

		/**
		* \brief Store block value to volatile memory.
		* \param blockno The block number.
		*/
		virtual void restore(unsigned char blockno);
    };
}

#endif /* LOGICALACCESS_MIFARESMARTIDCARDPROVIDER_HPP */