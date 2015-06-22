/**
 * \file mifarescmcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare scm card.
 */

#ifndef LOGICALACCESS_MIFARESCMCOMMANDS_HPP
#define LOGICALACCESS_MIFARESCMCOMMANDS_HPP

#include "mifarepcsccommands.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare commands class for SCM reader.
     * \remarks Firmware version must be >= 7.xx
     */
    class LIBLOGICALACCESS_API MifareSCMCommands : public MifarePCSCCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareSCMCommands();

        /**
         * \brief Destructor.
         */
        virtual ~MifareSCMCommands();

    protected:

        /**
         * \brief Load a key to the reader.
         * \param keyno The key number.
         * \param keytype The mifare key type.
         * \param key The key byte array.
         * \param keylen The key byte array length.
         * \param vol Use volatile memory.
         * \return true on success, false otherwise.
         */
        bool loadKey(unsigned char keyno, MifareKeyType keytype, const void* key, size_t keylen, bool vol = false);
        /**
         * \brief Authenticate a block, given a key number.
         * \param blockno The block number.
         * \param keyno The key number, previously loaded with Mifare::loadKey().
         * \param keytype The key type.
         */
        void authenticate(unsigned char blockno, unsigned char keyno, MifareKeyType keytype);

		/**
		* \brief Increment a block value.
		* \param blockno The block number.
		* \param value The increment value.
		*/
		virtual void increment(unsigned char blockno, unsigned int value);

		/**
		* \brief Decrement a block value.
		* \param blockno The block number.
		* \param value The decrement value.
		*/
		virtual void decrement(unsigned char blockno, unsigned int value);
    };
}

#endif /* LOGICALACCESS_MIFARESCMCOMMANDS_H */