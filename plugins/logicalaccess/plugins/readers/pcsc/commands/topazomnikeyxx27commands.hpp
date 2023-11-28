/**
* \file topazomnikeyxx27commands.hpp
* \author Maxime C. <maxime@leosac.com>
* \brief Topaz Omnikey XX27 PC/SC card.
*/

#ifndef LOGICALACCESS_TOPAZOMNIKEYXX27COMMANDS_HPP
#define LOGICALACCESS_TOPAZOMNIKEYXX27COMMANDS_HPP

#include <logicalaccess/plugins/cards/topaz/topazcommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_TOPAZOKXX27 "TopazOmnikeyXX27"
/**
* \brief The Topaz commands class for Omnikey XX27 PCSC reader.
*/
class LLA_READERS_PCSC_API TopazOmnikeyXX27Commands : public TopazCommands
{
  public:
    /**
    * \brief Constructor.
    */
    TopazOmnikeyXX27Commands();

    explicit TopazOmnikeyXX27Commands(std::string);

    /**
    * \brief Destructor.
    */
    virtual ~TopazOmnikeyXX27Commands();

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

#endif /* LOGICALACCESS_TOPAZOMNIKEYXX27COMMANDS_HPP */