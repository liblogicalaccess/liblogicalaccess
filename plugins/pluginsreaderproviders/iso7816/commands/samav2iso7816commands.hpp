/**
 * \file desfireiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include "samav2commands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"

#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{		
	#define DESFIRE_CLEAR_DATA_LENGTH_CHUNK	32
	/**
	 * \brief The DESFire base commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMAV2Commands
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV2ISO7816Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~SAMAV2ISO7816Commands();			

			virtual void		GetVersion();

			virtual void		GetKeyEntry();

			virtual void		ChangeKeyEntry();

			virtual void		ActiveAV2Mode();

			virtual void		AuthentificationHost();

			boost::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter()); };

			virtual std::string				GetSAMTypeFromSAM();

		protected:

			std::vector<unsigned char> sessionkey;

	};
}

#endif /* LOGICALACCESS_DESFIREISO7816COMMANDS_HPP */

