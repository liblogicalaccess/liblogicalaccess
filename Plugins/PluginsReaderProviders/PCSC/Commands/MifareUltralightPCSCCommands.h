/**
 * \file MifareUltralightPCSCCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight pcsc card.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTPCSCCOMMANDS_H
#define LOGICALACCESS_MIFAREULTRALIGHTPCSCCOMMANDS_H

#include "MifareUltralightCommands.h"
#include "../ReaderCardAdapters/PCSCReaderCardAdapter.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;


namespace LOGICALACCESS
{
	/**
	 * \brief The Mifare Ultralight commands class for PCSC reader.
	 */
	class LIBLOGICALACCESS_API MifareUltralightPCSCCommands : public MifareUltralightCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightPCSCCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightPCSCCommands();

			/**
			 * \brief Get the PC/SC reader/card adapter.
			 * \return The PC/SC reader/card adapter.
			 */
			boost::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() { return boost::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter()); };

			/**
			 * \brief Read a whole page.
			 * \param sector The page number, from 0 to 15.
			 * \param buf A buffer to fill with the data of the page.
			 * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t readPage(int page, void* buf, size_t buflen);

			/**
			 * \brief Write a whole page.
			 * \param sector The page number, from 0 to 15.
			 * \param buf A buffer to from which to copy the data.
			 * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
			 * \return The number of bytes written, or a negative value on error.
			 */
			virtual size_t writePage(int page, const void* buf, size_t buflen);
		
		protected:						
					
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTPCSCCOMMANDS_H */

