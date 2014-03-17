/**
 * \file SAMAV2ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include "samcommands.hpp"
#include "samav1iso7816commands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samcrypto.hpp"
#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{
	/**
	 * \brief The DESFire base commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMAV1ISO7816Commands
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

			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno);

			virtual boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > getKeyEntry(unsigned char keyno);
			virtual boost::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno);

			virtual void changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key);
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, boost::shared_ptr<DESFireKey> key);

		protected:

			void SAMAV2ISO7816Commands::generateSessionKey(std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2);

			std::vector<unsigned char> d_macSessionKey;

			unsigned char d_cmdCtr;
	};
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */

