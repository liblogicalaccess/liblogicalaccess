/**
 * \file rplethlcddisplay.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth LCD Display.
 */

#ifndef D3L_PCSC_RPLETHLCDDISPLAY_HPP
#define D3L_PCSC_RPLETHLCDDISPLAY_HPP

#include "logicalaccess/readerproviders/lcddisplay.hpp"
#include "readercardadapters/rplethreadercardadapter.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief A Rpleth LCD Display class.
     */
    class LIBLOGICALACCESS_API RplethLCDDisplay : public LCDDisplay
    {
    public:

        /**
         * \brief Constructor.
         */
        RplethLCDDisplay();

        /**
         * Does nothing.
         */
        virtual void clear() override;

         /**
         * \brief Write a message on screen.
         * \param message The message to show.
         */
        virtual void setMessage(std::string message);

        /**
         * \brief Write a message on screen during a time.
         * \param message The message to show.
         * \param time The time to show the message
         */
        virtual void setMessage(std::string message, int time);

        /**
         * \brief Write a message on screen.
         * \param rowid The row id for the message.
         * \param message The message to show.
         */
        virtual void setMessage(unsigned char rowid, std::string message);

        /**
         * \brief Set the default time to display a message.
         * \param time The time to show the message
         */
        void setDisplayTime(int time);

        /**
         * \brief Enable or disable the blink.
         */
        void blink();

        /**
         * \brief Enable or disable the autoScroll.
         */
        void autoScroll();

        /**
         * \brief Get the rpleth reader card adapter.
         * \return The rpleth reader card adapter.
         */
        std::shared_ptr<RplethReaderCardAdapter> getRplethReaderCardAdapter() { return std::dynamic_pointer_cast<RplethReaderCardAdapter>(getReaderCardAdapter()); };
    };
}

#endif /* D3L_PCSC_RPLETHLCDDISPLAY_HPP */