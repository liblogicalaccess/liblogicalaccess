/**
 * \file mifareultralightok5553commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare Ultralight OK5553 Commands
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcommands.hpp>
#include <logicalaccess/plugins/readers/ok5553/readercardadapters/ok5553readercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTOK5553 "MifareUltralightOK5553"

/**
 * \brief A Mifare Ultraligth OK5553 commands base class.
 */
class LLA_READERS_OK5553_API MifareUltralightOK5553Commands
    : public MifareUltralightCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareUltralightOK5553Commands();

    explicit MifareUltralightOK5553Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifareUltralightOK5553Commands();

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

    /**
     * \brief Get the OK5553 reader/card adapter.
     * \return The OK5553 reader/card adapter.
     */
    virtual std::shared_ptr<OK5553ReaderCardAdapter> getOK5553ReaderCardAdapter()
    {
        return std::dynamic_pointer_cast<OK5553ReaderCardAdapter>(getReaderCardAdapter());
    }
};
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTOK5553COMMANDS_HPP */