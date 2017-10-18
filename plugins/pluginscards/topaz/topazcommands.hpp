/**
 * \file topazcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz commands.
 */

#ifndef LOGICALACCESS_TOPAZCOMMANDS_HPP
#define LOGICALACCESS_TOPAZCOMMANDS_HPP

#include "topazaccessinfo.hpp"
#include "topazlocation.hpp"
#include "logicalaccess/cards/commands.hpp"

namespace logicalaccess
{
#define CMD_TOPAZ "Topaz"

    class TopazChip;

    /**
     * \brief The Topaz commands class.
     */
    class LIBLOGICALACCESS_API TopazCommands : public Commands
    {
    public:
		TopazCommands() : Commands(CMD_TOPAZ) {}

        explicit TopazCommands(std::string ct) : Commands(ct) {}

        /**
         * \brief Read several pages.
         * \param start_page The start page number, from 0 to stop_page.
         * \param stop_page The stop page number, from start_page to 15.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual ByteVector readPages(int start_page, int stop_page);

        /**
         * \brief Write several pages.
         * \param start_page The start page number, from 0 to stop_page.
         * \param stop_page The stop page number, from start_page to 15.
         * \param buf The buffer to fill with the data.
         * \param buflen The length of buf. Must be at least (stop_page - start_page + 1) * 4 bytes long.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual void writePages(int start_page, int stop_page, const ByteVector& buf);

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
        virtual ByteVector readPage(int page) = 0;

        /**
         * \brief Write a whole page.
         * \param sector The page number, from 0 to 15.
         * \param buf A buffer to from which to copy the data.
         * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
         * \return The number of bytes written, or a negative value on error.
         */
        virtual void writePage(int page, const ByteVector& buf) = 0;

    protected:

        std::shared_ptr<TopazChip> getTopazChip() const;
    };
}

#endif