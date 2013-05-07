/**
 * \file MifareUltralightCPCSCCommands.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C pcsc card.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_H
#define LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_H

#include "MifareUltralightPCSCCommands.h"
#include "MifareUltralightCCommands.h"


namespace LOGICALACCESS
{
	/**
	 * \brief The Mifare Ultralight C commands class for PCSC reader.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCPCSCCommands : public MifareUltralightPCSCCommands, public MifareUltralightCCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightCPCSCCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightCPCSCCommands();

			/**
			 * \brief Authenticate to the chip.
			 * \param authkey The authentication key.
			 */
			virtual void authenticate(boost::shared_ptr<TripleDESKey> authkey);

		
		protected:						
					
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_H */

