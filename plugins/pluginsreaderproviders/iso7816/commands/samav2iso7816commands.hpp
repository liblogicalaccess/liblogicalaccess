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
#include "samcrypto.hpp"
#include "samav2keyentry.hpp"

#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{		

	struct SAMManufactureInformation
	{
		unsigned char	uniqueserialnumber[7];
		unsigned char	productionbatchnumber[5];
		unsigned char	dayofproduction;
		unsigned char	monthofproduction;
		unsigned char	yearofproduction;
		unsigned char	globalcryptosettings;
		unsigned char	modecompatibility;
	};

	struct SAMVersionInformation
	{
		unsigned char	vendorid;
		unsigned char	type;
		unsigned char	subtype;
		unsigned char	majorversion;
		unsigned char	minorversion;
		unsigned char	storagesize;
		unsigned char	protocoltype;
	};

	struct SAMVersion
	{
		SAMVersionInformation		hardware;
		SAMVersionInformation		software;
		SAMManufactureInformation	manufacture;
	};
	
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

			virtual boost::shared_ptr<SAMAV2KeyEntry>		GetKeyEntry(unsigned int keyno);

			virtual void		ChangeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> key);

			virtual void		ActiveAV2Mode();

			virtual void		AuthentificationHost();

			boost::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter()); };

			virtual std::string				GetSAMTypeFromSAM();

		protected:

			std::vector<unsigned char> sessionkey;
	};
}

#endif /* LOGICALACCESS_DESFIREISO7816COMMANDS_HPP */

