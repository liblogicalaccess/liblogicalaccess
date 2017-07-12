/**
 * \file mifareultralightccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C commands.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP

#include "mifareultralightcommands.hpp"
#include "mifareultralightcaccessinfo.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTC		"MifareUltralightC"

	class MifareUltralightCChip;

    /**
     * \brief The Mifare Ultralight C commands class.
     */
    class LIBLOGICALACCESS_API MifareUltralightCCommands : public virtual MifareUltralightCommands
    {
    public:

		MifareUltralightCCommands() : MifareUltralightCommands(CMD_MIFAREULTRALIGHTC) {}

		MifareUltralightCCommands(std::string ct) : MifareUltralightCommands(ct) {}

        /**
         * \brief Authenticate to the chip.
         * \param aiToUse The access information to use for authentication.
         */
        void authenticate(std::shared_ptr<AccessInfo> aiToUse);

        /**
         * \brief Change the chip authentication key.
         * \param aiToWrite The access information to write.
         */
        void changeKey(std::shared_ptr<AccessInfo> aiToWrite);

        /**
         * \brief Change the chip authentication key.
         * \param key The new key.
         */
        virtual void changeKey(std::shared_ptr<TripleDESKey> key);

        /**
         * \brief Set a page as read-only.
         * \param page The page to lock.
         */
        virtual void lockPage(int page);

        /**
         * \brief Authenticate to the chip.
         * \param authkey The authentication key.
         */
        virtual void authenticate(std::shared_ptr<TripleDESKey> authkey) = 0;

	protected:

		std::shared_ptr<MifareUltralightCChip> getMifareUltralightCChip();
    };
}

#endif