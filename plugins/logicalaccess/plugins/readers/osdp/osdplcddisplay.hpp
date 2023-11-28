/**
* \file osdplcddisplay.hpp
* \author Maxime C. <maxime@leosac.com>
* \brief OSDP LCD Display.
*/

#ifndef LOGICALACCESS_OSDPLCDDISPLAY_HPP
#define LOGICALACCESS_OSDPLCDDISPLAY_HPP

#include <logicalaccess/readerproviders/lcddisplay.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpreadercardadapter.hpp>

#include <cstring>
#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{

/**
* \brief A OSDP LCD Display class.
*/
class LLA_READERS_OSDP_API OSDPLCDDisplay : public LCDDisplay
{
  public:
    /**
    * \brief Constructor.
    */
    OSDPLCDDisplay();

    /**
    * \brief Write a message on screen.
    * \param message The message to show.
    */
    void setMessage(std::string message) override;

    /**
    * \brief Write a message on screen.
    * \param rowid The row id for the message.
    * \param message The message to show.
    */
    void setMessage(unsigned char rowid, std::string message) override;

    std::shared_ptr<OSDPReaderCardAdapter> getOSDPReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<OSDPReaderCardAdapter>(getReaderCardAdapter());
    }

    /**
     * The implementation simply write to line of SPACE character.
     */
    void clear() override;
};
}

#endif /* LOGICALACCESS_OSDPLCDDISPLAY_HPP */
