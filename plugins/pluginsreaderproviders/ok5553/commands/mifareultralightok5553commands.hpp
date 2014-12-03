/**
 * \file mifareultralightok5553commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight OK5553 Commands
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP

#include "mifareultralightcommands.hpp"
#include "../readercardadapters/ok5553readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief A Mifare Ultraligth OK5553 commands base class.
     */
    class LIBLOGICALACCESS_API MifareUltralightOK5553Commands : public MifareUltralightCommands
    {
    public:
        /**
         * \brief Constructor.
         */
        MifareUltralightOK5553Commands();

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightOK5553Commands();

        /**
         * \brief Read a whole page.
         * \param sector The page number, from 0 to 15.
         * \param buf A buffer to fill with the data of the page.
         * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
         * \return The number of bytes red, or a negative value on error.
         */
        virtual std::vector<unsigned char> readPage(int page);

        /**
         * \brief Write a whole page.
         * \param sector The page number, from 0 to 15.
         * \param buf A buffer to from which to copy the data.
         * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
         * \return The number of bytes written, or a negative value on error.
         */
        virtual void writePage(int page, const std::vector<unsigned char>& buf);

        /**
         * \brief Get the OK5553 reader/card adapter.
         * \return The OK5553 reader/card adapter.
         */
        virtual std::shared_ptr<OK5553ReaderCardAdapter> getOK5553ReaderCardAdapter() { return std::dynamic_pointer_cast<OK5553ReaderCardAdapter>(getReaderCardAdapter()); };
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP */