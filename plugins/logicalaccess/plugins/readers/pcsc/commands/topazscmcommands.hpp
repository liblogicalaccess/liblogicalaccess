/**
* \file topazscmcommands.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz SCM PC/SC card.
*/

#ifndef LOGICALACCESS_TOPAZSCMCOMMANDS_HPP
#define LOGICALACCESS_TOPAZSCMCOMMANDS_HPP

#include <logicalaccess/plugins/cards/topaz/topazcommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_TOPAZSCM "TopazSCM"
/**
* \brief The Topaz commands class for SCM PCSC reader.
*/
class LIBLOGICALACCESS_API TopazSCMCommands : public TopazCommands
{
  public:
    /**
    * \brief Constructor.
    */
    TopazSCMCommands();

    explicit TopazSCMCommands(std::string);

    /**
    * \brief Destructor.
    */
    virtual ~TopazSCMCommands();

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

#endif /* LOGICALACCESS_TOPAZSCMCOMMANDS_HPP */