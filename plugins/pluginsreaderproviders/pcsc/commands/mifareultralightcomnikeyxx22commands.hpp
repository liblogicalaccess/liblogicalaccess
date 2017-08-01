/**
* \file mifareultralightcomnikeyxx22commands.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Mifare Ultralight C - Omnikey xx22.
*/

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX22COMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX22COMMANDS_HPP

#include "mifareultralightcpcsccommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCOMNIKEYXX22 "MifareUltralightCOmnikeyXX22"

    /**
    * \brief The Mifare Ultralight C commands class for Omnikey xx22 reader.
    */
    class LIBLOGICALACCESS_API MifareUltralightCOmnikeyXX22Commands : public MifareUltralightCCommands
	{
    public:

        /**
        * \brief Constructor.
        */
        MifareUltralightCOmnikeyXX22Commands();

		MifareUltralightCOmnikeyXX22Commands(std::string);

        /**
        * \brief Destructor.
        */
        virtual ~MifareUltralightCOmnikeyXX22Commands();

        /**
        * \brief Authenticate to the chip.
        * \param authkey The authentication key.
        */
        virtual void authenticate(std::shared_ptr<TripleDESKey> authkey);

		/**
		* \brief Get the PC/SC reader/card adapter.
		* \return The PC/SC reader/card adapter.
		*/
		std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() { return bridge->getPCSCReaderCardAdapter(); }

		/**
		* \brief Read a whole page.
		* \param sector The page number, from 0 to 15.
		* \param buf A buffer to fill with the data of the page.
		* \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
		* \return The number of bytes red, or a negative value on error.
		*/
		virtual std::vector<unsigned char> readPage(int page) { return bridge->readPage(page); }

		/**
		* \brief Write a whole page.
		* \param sector The page number, from 0 to 15.
		* \param buf A buffer to from which to copy the data.
		* \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
		* \return The number of bytes written, or a negative value on error.
		*/
		virtual void writePage(int page, const std::vector<unsigned char>& buf) { return bridge->writePage(page, buf); }

	private:

		std::unique_ptr<MifareUltralightPCSCCommands> bridge;
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX22COMMANDS_HPP */