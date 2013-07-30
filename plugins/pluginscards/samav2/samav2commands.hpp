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
#include "desfirekey.hpp"

namespace logicalaccess
{
	class LIBLOGICALACCESS_API SAMAV2Commands : public virtual Commands
	{
		public:		
			virtual void									GetVersion() = 0;
			virtual boost::shared_ptr<SAMAV2KeyEntry>		GetKeyEntry(unsigned int keyno) = 0;
			virtual boost::shared_ptr<SAMAV2KUCEntry>		GetKUCEntry(unsigned int keyno) = 0;
			virtual void									ChangeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KeyEntry> key) = 0;
			virtual void									ChangeKUCEntry(unsigned char keyno, boost::shared_ptr<SAMAV2KUCEntry> key) = 0;
			virtual void									ActiveAV2Mode() = 0;
			virtual void									AuthentificationHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno) = 0;
			virtual std::string								GetSAMTypeFromSAM() = 0;
			virtual void									DisableKeyEntry(unsigned char keyno) = 0;
			virtual void									SelectApplication(unsigned char *aid) = 0;
		protected:
			
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */

