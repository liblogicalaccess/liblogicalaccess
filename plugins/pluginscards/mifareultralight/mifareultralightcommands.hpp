/**
 * \file mifareultralightcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight commands.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCOMMANDS_HPP

#include "mifareultralightaccessinfo.hpp"
#include "mifareultralightlocation.hpp"
#include "logicalaccess/cards/commands.hpp"

namespace logicalaccess
{
    class MifareUltralightChip;

    /**
     * \brief The Mifare Ultralight commands class.
     */
    class LIBLOGICALACCESS_API MifareUltralightCommands : public virtual Commands
    {
    public:

        /**
         * \brief Read several pages.
         * \param start_page The start page number, from 0 to stop_page.
         * \param stop_page The stop page number, from start_page to 15.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual size_t readPages(int start_page, int stop_page, void* buf, size_t buflen);

        /**
         * \brief Write several pages.
         * \param start_page The start page number, from 0 to stop_page.
         * \param stop_page The stop page number, from start_page to 15.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual size_t writePages(int start_page, int stop_page, const void* buf, size_t buflen);

        /**
         * \brief Set a page as read-only.
         * \param page The page to lock.
         */
        virtual void lockPage(int page);

        /**
         * \brief Read a whole page.
         * \param sector The page number, from 0 to 15.
         * \param buf A buffer to fill with the data of the page.
         * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual size_t readPage(int page, void* buf, size_t buflen) = 0;

        /**
         * \brief Write a whole page.
         * \param sector The page number, from 0 to 15.
         * \param buf A buffer to from which to copy the data.
         * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
         * \return The number of bytes written, or a negative value on error.
         */
        virtual size_t writePage(int page, const void* buf, size_t buflen) = 0;

    protected:

        boost::shared_ptr<MifareUltralightChip> getMifareUltralightChip();
    };
}

#endif