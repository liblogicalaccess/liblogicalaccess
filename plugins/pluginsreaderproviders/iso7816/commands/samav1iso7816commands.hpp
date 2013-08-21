/**
 * \file desfireiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP

#include "samcommands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samcrypto.hpp"
#include "samcommands.hpp"

#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{		
	/**
	 * \brief The DESFire base commands class.
	 */
	class LIBLOGICALACCESS_API SAMAV1ISO7816Commands : public SAMCommands
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

			virtual boost::shared_ptr<SAMVersion> getVersion();

			virtual boost::shared_ptr<SAMKeyEntry> getKeyEntry(unsigned char keyno);
			virtual boost::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno);

			virtual void changeKUCEntry(unsigned char kucno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key);
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry> keyentry, boost::shared_ptr<DESFireKey> key);

			virtual void activeAV2Mode();
			virtual void selectApplication(unsigned char *aid);
			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHost_AES_3K3DES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			void authentificateHostDES(boost::shared_ptr<DESFireKey> key, unsigned char keyno);
			virtual void disableKeyEntry(unsigned char keyno);
			virtual std::vector<unsigned char> dumpSessionKey();

			boost::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return boost::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter()); };
			virtual std::string getSAMTypeFromSAM();

			boost::shared_ptr<SAMDESfireCrypto> getCrypto() { return d_crypto; };
			void setCrypto(boost::shared_ptr<SAMDESfireCrypto> t) { d_crypto = t; };

			virtual std::vector<unsigned char> decipherData(std::vector<unsigned char> data, bool islastdata);
			virtual std::vector<unsigned char> encipherData(std::vector<unsigned char> data, bool islastdata);

			virtual std::vector<unsigned char> changeKeyPICC(boost::shared_ptr<changeKeyInfo> info);

		protected:
			boost::shared_ptr<SAMDESfireCrypto> d_crypto;
	};
}

#endif /* LOGICALACCESS_DESFIREISO7816COMMANDS_HPP */

