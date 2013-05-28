/**
 * \file mifareultralightrplethcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight Rpleth Commands
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTRPLETHCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTRPLETHCOMMANDS_HPP

#include "mifareultralightcommands.hpp"
#include "../readercardadapters/rplethreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	/**
	 * \brief A Mifare Ultraligth Rpleth commands base class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightRplethCommands : public MifareUltralightCommands
	{
	public:
			/**
			 * \brief Constructor.
			 */
			MifareUltralightRplethCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightRplethCommands();
			
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
			
			/**
			 * \brief Get the Rpleth reader/card adapter.
			 * \return The Rpleth reader/card adapter.
			 */
			virtual boost::shared_ptr<RplehtReaderCardAdapter> getRplethReaderCardAdapter() { return boost::dynamic_pointer_cast<RplehtReaderCardAdapter>(getReaderCardAdapter()); };
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTRPLETHCOMMANDS_HPP */

