/**
 * \file a3mlgm5600lcddisplay.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 LCD Display.
 */

#ifndef LOGICALACCESS_A3MLGM5600LCDDISPLAY_HPP
#define LOGICALACCESS_A3MLGM5600LCDDISPLAY_HPP

#include "logicalaccess/readerproviders/lcddisplay.hpp"
#include "readercardadapters/a3mlgm5600readercardadapter.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
/**
 * \brief A A3MLGM5600 LCD Display class.
 */
class LIBLOGICALACCESS_API A3MLGM5600LCDDisplay : public LCDDisplay
{
  public:
    /**
     * \brief Constructor.
     */
    A3MLGM5600LCDDisplay();

    /**
     * \brief Write a message on screen.
     * \param message The message to show.
     */
    void setMessage(std::string message) override;


    void clear() override;

    /**
     * \brief Write a message on screen.
     * \param rowid The row id for the message.
     * \param message The message to show.
     */
    void setMessage(unsigned char rowid, std::string message) override;

    std::shared_ptr<A3MLGM5600ReaderCardAdapter> getA3MLGM5600ReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<A3MLGM5600ReaderCardAdapter>(
            getReaderCardAdapter());
    }
};
}

#endif /* LOGICALACCESS_A3MLGM5600LCDDISPLAY_HPP */