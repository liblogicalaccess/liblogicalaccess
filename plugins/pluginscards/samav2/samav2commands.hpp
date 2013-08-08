/**
 * \file desfireev1commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 commands.
 */

#ifndef LOGICALACCESS_SAMAV2COMMANDS_HPP
#define LOGICALACCESS_SAMAV2COMMANDS_HPP

#include "logicalaccess/xmlserializable.hpp"
#include "logicalaccess/cards/commands.hpp"
#include "samav2keyentry.hpp"
#include "samav2kucentry.hpp"
#include "desfirekey.hpp"

namespace logicalaccess
{
	class LIBLOGICALACCESS_API SAMAV2Commands : public virtual Commands
	{
		public:		
			virtual void getVersion() = 0;
			virtual boost::shared_ptr<SAMAV2KeyEntry> getKeyEntry(unsigned int keyno) = 0;
			virtual boost::shared_ptr<SAMAV2KucEntry> getKUCEntry(unsigned int keyno) = 0;
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> keyentry, boost::shared_ptr<DESFireKey> key) = 0;
			virtual void changeKUCEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KucEntry> keyentry, boost::shared_ptr<DESFireKey> key) = 0;
			virtual void activeAV2Mode() = 0;
			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno) = 0;
			virtual std::string	getSAMTypeFromSAM() = 0;
			virtual void disableKeyEntry(unsigned char keyno) = 0;
			virtual void selectApplication(unsigned char *aid) = 0;
			virtual std::vector<unsigned char> dumpSessionKey() = 0;
		protected:
			
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */

