/**
* \file topazpcsccommands.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Topaz pcsc card.
*/

#ifndef LOGICALACCESS_TOPAZPCSCCOMMANDS_HPP
#define LOGICALACCESS_TOPAZPCSCCOMMANDS_HPP

#include "topaz/topazcommands.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_TOPAZPCSC "TopazPCSC"
    /**
    * \brief The Topaz commands class for PCSC reader.
    */
    class LIBLOGICALACCESS_API TopazPCSCCommands : public TopazCommands
    {
    public:

        /**
        * \brief Constructor.
        */
        TopazPCSCCommands();

        explicit TopazPCSCCommands(std::string);

        /**
        * \brief Destructor.
        */
        virtual ~TopazPCSCCommands();

        /**
        * \brief Get the PC/SC reader/card adapter.
        * \return The PC/SC reader/card adapter.
        */
        std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() const { return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter()); }

        /**
        * \brief Read a whole page.
        * \param sector The page number, from 0 to 15.
        * \param buf A buffer to fill with the data of the page.
        * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
        * \return The number of bytes red, or a negative value on error.
        */
	    ByteVector readPage(int page) override;

        /**
        * \brief Write a whole page.
        * \param sector The page number, from 0 to 15.
        * \param buf A buffer to from which to copy the data.
        * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
        * \return The number of bytes written, or a negative value on error.
        */
	    void writePage(int page, const ByteVector& buf) override;
    };
}

#endif /* LOGICALACCESS_TOPAZPCSCCOMMANDS_HPP */