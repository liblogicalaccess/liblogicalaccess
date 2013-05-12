/**
 * \file mifareprofile.hpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare card profiles.
 */

#ifndef LOGICALACCESS_MIFARE_HPP
#define LOGICALACCESS_MIFARE_HPP

#include "logicalaccess/cards/profile.hpp"
#include "mifareaccessinfo.hpp"
#include "mifarelocation.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{	
	/**
	 * \brief The Mifare base profile class.
	 */
	class LIBLOGICALACCESS_API MifareProfile : public Profile
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareProfile();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareProfile();		

			/**
			 * \brief Get one of the Mifare keys of this profile.
			 * \param index The index of the key, from 0 to 15.
			 * \param keytype The key type.
			 * \return The specified mifare key or a null key if either index or keytype is invalid.
			 */
			boost::shared_ptr<MifareKey> getKey(int index, MifareKeyType keytype) const;

			/**
			 * \brief Set one of the Mifare keys of this profile.
			 * \param index The index of the key to set, from 0 to 15.
			 * \param keytype The key type.
			 * \param key The value of the key.
			 * \warning setKey() automatically calls setMifareKeyUsage(index, keytype, true).
			 */
			void setKey(int index, MifareKeyType keytype, boost::shared_ptr<MifareKey> key);			
			
			/**
			 * \brief Set default keys for the mifare card in memory.
			 */
			virtual void setDefaultKeys();

			/**
			 * \brief Set default keys for the type card in memory at a specific location.
			 */
			virtual void setDefaultKeysAt(boost::shared_ptr<Location> location);

			/**
			 * \brief Clear all keys in memory.
			 */
			virtual void clearKeys();

			/**
			 * \brief Set Mifare key at a specific location.
			 * \param location The key's location.
			 * \param AccessInfo The key's informations.
			 */
			virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo);

			/**
			 * \brief Set default keys for a sector.
			 * \param index The sector index.
			 */
			void setDefaultKeysAt(int index);			

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
			bool getKeyUsage(int index, MifareKeyType keytype) const;

			/**
			 * \brief Set one of the Mifare keys of this profile.
			 * \param index The index of the key to set
			 * \param keytype The key type.
			 * \param used True if the key is used, false otherwise.
			 */
			void setKeyUsage(int index, MifareKeyType keytype, bool used);
			
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

			/**
			 * \brief The real keys, + 2 for new keys.
			 */
			boost::shared_ptr<MifareKey>* d_key;
	};	
}

#endif /* LOGICALACCESS_MIFARE_HPP */

