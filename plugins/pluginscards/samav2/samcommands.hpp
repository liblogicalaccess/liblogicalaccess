/**
 * \file SAMCommands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMCommands commands.
 */

#ifndef LOGICALACCESS_SAMCOMMANDS_HPP
#define LOGICALACCESS_SAMCOMMANDS_HPP

#include "logicalaccess/xmlserializable.hpp"
#include "logicalaccess/cards/commands.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"

namespace logicalaccess
{
	typedef struct s_SAMManufactureInformation
	{
		unsigned char	uniqueserialnumber[7];
		unsigned char	productionbatchnumber[5];
		unsigned char	dayofproduction;
		unsigned char	monthofproduction;
		unsigned char	yearofproduction;
		unsigned char	globalcryptosettings;
		unsigned char	modecompatibility;
	} SAMManufactureInformation;

	typedef struct s_SAMVersionInformation
	{
		unsigned char	vendorid;
		unsigned char	type;
		unsigned char	subtype;
		unsigned char	majorversion;
		unsigned char	minorversion;
		unsigned char	storagesize;
		unsigned char	protocoltype;
	} SAMVersionInformation;

	typedef struct s_SAMVersion
	{
		SAMVersionInformation		hardware;
		SAMVersionInformation		software;
		SAMManufactureInformation	manufacture;
	} SAMVersion;

	enum SAMLockUnlock {
			Unlock = 0x00,
			LockWithoutSpecifyingKey = 0x01,
			LockWithSpecifyingKey = 0x02,
			SwitchAV2Mode = 0x03
	};

	class DESFireKey;

	class LIBLOGICALACCESS_API SAMCommands : public virtual Commands
	{
		public:		
			virtual SAMVersion getVersion() = 0;
			virtual boost::shared_ptr<SAMKeyEntry> getKeyEntry(unsigned char keyno) = 0;
			virtual boost::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char keyno) = 0;
			virtual void changeKeyEntry(unsigned char keyno, boost::shared_ptr<SAMKeyEntry> keyentry, boost::shared_ptr<DESFireKey> key) = 0;
			virtual void changeKUCEntry(unsigned char keyno, boost::shared_ptr<SAMKucEntry> keyentry, boost::shared_ptr<DESFireKey> key) = 0;
			virtual void authentificateHost(boost::shared_ptr<DESFireKey> key, unsigned char keyno) = 0;
			virtual std::string	getSAMTypeFromSAM() = 0;
			virtual void disableKeyEntry(unsigned char keyno) = 0;
			virtual void selectApplication(unsigned char *aid) = 0;
			virtual std::vector<unsigned char> dumpSessionKey() = 0;
			virtual std::vector<unsigned char> decipherData(std::vector<unsigned char> data, bool islastdata) = 0;
			virtual std::vector<unsigned char> encipherData(std::vector<unsigned char> data, bool islastdata) = 0;
			virtual std::vector<unsigned char> changeKeyPICC(const ChangeKeyInfo& info) = 0;
			virtual void lockUnlock(boost::shared_ptr<DESFireKey> masterKey, SAMLockUnlock state, unsigned char keyno, unsigned char unlockkeyno, unsigned char unlockkeyversion) = 0;	
		protected:
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */

