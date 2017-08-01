/**
 * \file mifareultralightcpcsccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C pcsc card.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP

#include "mifareultralightpcsccommands.hpp"
#include "mifareultralight/mifareultralightccommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCPCSC "MifareUltralightCPCSC"
    /**
     * \brief The Mifare Ultralight C commands class for PCSC reader.
     */
    class LIBLOGICALACCESS_API MifareUltralightCPCSCCommands : public MifareUltralightCCommands  
	{
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightCPCSCCommands();

		MifareUltralightCPCSCCommands(std::string);


        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCPCSCCommands();

        /**
         * \brief Authenticate to the chip.
         * \param authkey The authentication key.
         */
        virtual void authenticate(std::shared_ptr<TripleDESKey> authkey);

		/**
		* \brief Get the PC/SC reader/card adapter.
		* \return The PC/SC reader/card adapter.
		*/
		std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() {  return bridgeMFU->getPCSCReaderCardAdapter(); }

		/**
		* \brief Read a whole page.
		* \param sector The page number, from 0 to 15.
		* \param buf A buffer to fill with the data of the page.
		* \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
		* \return The number of bytes red, or a negative value on error.
		*/
		virtual std::vector<unsigned char> readPage(int page) {  return bridgeMFU->readPage(page); }

		/**
		* \brief Write a whole page.
		* \param sector The page number, from 0 to 15.
		* \param buf A buffer to from which to copy the data.
		* \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
		* \return The number of bytes written, or a negative value on error.
		*/
		virtual void writePage(int page, const std::vector<unsigned char>& buf) {  return bridgeMFU->writePage(page, buf); }

    protected:

		virtual void startGenericSession();

		virtual void stopGenericSession();

        virtual std::vector<unsigned char> sendGenericCommand(const std::vector<unsigned char>& data);

        virtual std::vector<unsigned char> authenticate_PICC1();

        virtual std::vector<unsigned char> authenticate_PICC2(const std::vector<unsigned char>& encRndAB);

	private:

		std::shared_ptr<MifareUltralightPCSCCommands> bridgeMFU;
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP */