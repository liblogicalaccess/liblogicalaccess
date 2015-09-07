/**
 * \file lcddisplay.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LCD Display.
 */

#ifndef LOGICALACCESS_LCDDISPLAY_HPP
#define LOGICALACCESS_LCDDISPLAY_HPP

#include "logicalaccess/xmlserializable.hpp"

namespace logicalaccess
{
    class ReaderCardAdapter;

    /**
     * \brief A LCD Display base class to display message on LCD screen.
     */
    class LIBLOGICALACCESS_API LCDDisplay
    {
    public:

        /**
         * \brief Write a message on screen.
         * \param message The message to show.
         */
        virtual void setMessage(std::string message) = 0;

        /**
         * \brief Write a message on screen.
         * \param rowid The row id for the message.
         * \param message The message to show.
         */
        virtual void setMessage(unsigned char rowid, std::string message) = 0;

        /**
         * Clear the LCD screen.
         *
         * @note The command is not implemented for Rpleth LCD display.
         */
        virtual void clear() = 0;

        /**
         * \brief Get the reader card adapter.
         * \return The reader card adapter.
         */
        std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; };

        /**
         * \brief Set the reader card adapter.
         * \param adapter The reader card adapter.
         */
        void setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; };

    protected:

        /**
         * \brief The reader card adapter.
         */
        std::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;
    };
}

#endif /* LOGICALACCESS_LCDDISPLAY_HPP */