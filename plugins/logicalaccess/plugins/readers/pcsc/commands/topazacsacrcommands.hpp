/**
* \file topazacsacrcommands.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz ACS ACR PC/SC card.
*/

#ifndef LOGICALACCESS_TOPAZACSACRCOMMANDS_HPP
#define LOGICALACCESS_TOPAZACSACRCOMMANDS_HPP

#include <logicalaccess/plugins/cards/topaz/topazcommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_TOPAZACSACR "TopazACSACR"

/**
* \brief The Topaz commands class for ACS ACR PCSC reader.
*/
class LLA_READERS_PCSC_API TopazACSACRCommands : public TopazCommands
{
  public:
    /**
    * \brief Constructor.
    */
    TopazACSACRCommands();

    explicit TopazACSACRCommands(std::string);

    /**
    * \brief Destructor.
    */
    virtual ~TopazACSACRCommands();

    /**
    * \brief Get the PC/SC reader/card adapter.
    * \return The PC/SC reader/card adapter.
    */
    std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter());
    }

    /**
    * \brief Read a whole page.
    * \param sector The page number, from 0 to 15.
    * \param buf A buffer to fill with the data of the page.
    * \param buflen The length of buffer. Must be at least 4 bytes long or the call will
    * fail.
    * \return The number of bytes red, or a negative value on error.
    */
    ByteVector readPage(int page) override;

    /**
    * \brief Write a whole page.
    * \param sector The page number, from 0 to 15.
    * \param buf A buffer to from which to copy the data.
    * \param buflen The length of buffer. Must be at least 4 bytes long or the call will
    * fail.
    * \return The number of bytes written, or a negative value on error.
    */
    void writePage(int page, const ByteVector &buf) override;
};
}

#endif /* LOGICALACCESS_TOPAZACSACRCOMMANDS_HPP */