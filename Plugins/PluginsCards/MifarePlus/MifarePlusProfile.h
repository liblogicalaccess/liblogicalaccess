/**
 * \file MifarePlusProfile.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#ifndef LOGICALACCESS_MIFAREPLUS_H
#define LOGICALACCESS_MIFAREPLUS_H

#include "logicalaccess/Cards/Profile.h"
#include "MifarePlusAccessInfo.h"
#include "MifarePlusLocation.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{	
	/**
	 * \brief The MifarePlus base profile class.
	 */
	class LIBLOGICALACCESS_API MifarePlusProfile : public Profile
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifarePlusProfile();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusProfile();	

			/**
			 * \brief Get one of the Mifare keys of this profile.
			 * \param index The index of the key, from 0 to 15 (unused if not sector key).
			 * \param keytype The key type.
			 * \return The specified mifare key or a null key if either index or keytype is invalid.
			 */
			virtual boost::shared_ptr<MifarePlusKey> getKey(int index, MifarePlusKeyType keytype) const = 0;

			/**
			 * \brief Create a new AES key.
			 */
			virtual inline boost::shared_ptr<MifarePlusKey> createAESKey() { return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE)); };

			/**
			 * \brief Create a new crypto1 key.
			 */
			virtual inline boost::shared_ptr<MifarePlusKey> createCrypto1Key() { return boost::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_CRYPTO1_KEY_SIZE)); };


			/**
			 * \brief Set one of the Mifare keys of this profile.
			 * \param index The index of the key to set, from 0 to 15.
			 * \param keytype The key type.
			 * \param key The value of the key.
			 * \warning setKey() automatically calls setMifareKeyUsage(index, keytype, true).
			 */
			virtual void setKey(int index, MifarePlusKeyType keytype, boost::shared_ptr<MifarePlusKey> key) = 0;			
			
			/**
			 * \brief Set default keys for the mifare card in memory.
			 */
			virtual void setDefaultKeys() = 0;

			/**
			 * \brief Set default keys for the type card in memory at a specific location.
			 */
			virtual void setDefaultKeysAt(boost::shared_ptr<Location> location);

			/**
			 * \brief Clear all keys in memory.
			 */
			virtual void clearKeys() = 0;

			/**
			 * \brief Set Mifare key at a specific location.
			 * \param location The key's location.
			 * \param AccessInfo The key's informations.
			 */
			virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo) = 0;

			/**
			 * \brief Set default keys for a sector.
			 * \param index The sector index.
			 */
			virtual void setDefaultKeysAt(int index) = 0;			

			/**
			 * \brief Create default Mifare access informations.
			 * \return Default Mifare access informations.
			 */
			virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

			/**
			 * \brief Create default Mifare location.
			 * \return Default Mifare location.
			 */
			virtual boost::shared_ptr<Location> createLocation() const;	

			/**
			 * \brief Get one of the Mifare keys usage.
			 * \param index The index of the key
			 * \param keytype The keytype.
			 * \return true if the key is used, false otherwise.
			 */
			virtual bool getKeyUsage(int index, MifarePlusKeyType keytype) const = 0;

			/**
			 * \brief Set one of the Mifare keys of this profile.
			 * \param index The index of the key to set
			 * \param keytype The key type.
			 * \param used True if the key is used, false otherwise.
			 */
			virtual void setKeyUsage(int index, MifarePlusKeyType keytype, bool used) = 0;
			
			/**
			 * \brief Get the supported format list.
			 * \return The format list.
			 */
			virtual FormatList getSupportedFormatList();

		protected:	

			/**
			 * \brief Get the max nb sectors.
			 * \return The max nb sectors.
			 */
			unsigned int getNbSectors() const;
	};	
}

#endif /* LOGICALACCESS_MIFAREPLUS_H */
