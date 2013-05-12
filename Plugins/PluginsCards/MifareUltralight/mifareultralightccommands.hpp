/**
 * \file mifareultralightccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C commands.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP

#include "mifareultralightcommands.hpp"
#include "mifareultralightcaccessinfo.hpp"


namespace logicalaccess
{
	/**
	 * \brief The Mifare Ultralight C commands class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCCommands : public virtual Commands
	{
		public:

			/**
			 * \brief Authenticate to the chip.
			 * \param authkey The authentication key.
			 */
			virtual void authenticate(boost::shared_ptr<TripleDESKey> authkey) = 0;
	};
}

#endif
