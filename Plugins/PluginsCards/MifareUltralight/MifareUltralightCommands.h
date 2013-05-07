/**
 * \file MifareUltralightCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight commands.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCOMMANDS_H
#define LOGICALACCESS_MIFAREULTRALIGHTCOMMANDS_H

#include "MifareUltralightAccessInfo.h"
#include "MifareUltralightLocation.h"
#include "logicalaccess/Cards/Commands.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The Mifare Ultralight commands class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCommands : public virtual Commands
	{
		public:

			/**
			 * \brief Read a whole page.
			 * \param sector The page number, from 0 to 15.
			 * \param buf A buffer to fill with the data of the page.
			 * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
			 * \return The number of bytes red, or a negative value on error.
			 */
			virtual size_t readPage(int page, void* buf, size_t buflen) = 0;

			/**
			 * \brief Write a whole page.
			 * \param sector The page number, from 0 to 15.
			 * \param buf A buffer to from which to copy the data.
			 * \param buflen The length of buffer. Must be at least 4 bytes long or the call will fail.
			 * \return The number of bytes written, or a negative value on error.
			 */
			virtual size_t writePage(int page, const void* buf, size_t buflen) = 0;
	};
}

#endif
