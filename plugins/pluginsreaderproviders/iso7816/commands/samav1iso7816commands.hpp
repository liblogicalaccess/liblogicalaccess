/**
 * \file SAMAV1ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samiso7816commands.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samcrypto.hpp"
#include "samcommands.hpp"

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#include <string>
#include <vector>
#include <iostream>

#define DEFAULT_SAM_CLA 0x80

namespace logicalaccess
{		
	/**
	 * \brief The SAMAV1ISO7816 commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV1ISO7816Commands : public SAMISO7816Commands<KeyEntryAV1Information, SETAV1>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			SAMAV1ISO7816Commands();

			/**
			 * \brief Destructor.
			 */
			virtual ~SAMAV1ISO7816Commands();			

			virtual boost::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1> > getKeyEntry(unsigned char keyno);
			virtual boost::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno);

			virtual void changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key);
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1> > keyentry, boost::shared_ptr<DESFireKey> key);

			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);

			virtual std::vector<unsigned char> changeKeyPICC(const ChangeKeyInfo& info);

		protected:

	};
}

#endif /* LOGICALACCESS_SAMAV1ISO7816COMMANDS_HPP */

