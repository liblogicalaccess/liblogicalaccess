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
#define CMD_MIFARESCM "MifareSCM"

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

        explicit MifareSCMCommands(std::string);

        /**
         * \brief Destructor.
         */
        virtual ~MifareSCMCommands();

    protected:

        /**
         * \brief Load a key to the reader.
         * \param keyno The key number.
         * \param keytype The mifare key type.
         * \param key The key.
         * \param vol Use volatile memory.
         * \return true on success, false otherwise.
         */
        bool loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol = false) override;

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
    };
}

#endif /* LOGICALACCESS_MIFARESCMCOMMANDS_H */