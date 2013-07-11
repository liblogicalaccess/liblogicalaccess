/**
 * \file mifarestoragecardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare storage card service.
 */

#ifndef LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP
#define LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP

#include "logicalaccess/key.hpp"
#include "mifarelocation.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "mifarechip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{	
	/**
	 * \brief The Mifare storage card service base class.
	 */
	class LIBLOGICALACCESS_API MifareStorageCardService : public StorageCardService
	{
		public:			

			/**
			 * \brief Constructor.
			 * \param chip The chip.
			 */
			MifareStorageCardService(boost::shared_ptr<Chip> chip);

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareStorageCardService();

			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase();

			/**
			 * \brief Erase a specific location on the card.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use to delete.
			 * \return true if the card was erased, false otherwise. If false, the card may be partially erased.
			 */
			virtual bool erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse);

			/**
			 * \brief Write data on a specific Mifare location, using given Mifare keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param aiToWrite The key's informations to change.
			 * \param data Data to write.
			 * \param dataLength Data's length to write.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags);

			/**
			 * \brief Read data on a specific Mifare location, using given Mifare keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags);			

			/**
			 * \brief Read data header on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \return Data header length.
			 */
			virtual size_t readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength);

		protected:							

			/*
			 * \brief Get the associated Mifare chip.
			 * \return The Mifare chip.
			 */
			boost::shared_ptr<MifareChip> getMifareChip() { return boost::dynamic_pointer_cast<MifareChip>(getChip()); };
	};	
}

#endif /* LOGICALACCESS_MIFARESTORAGECARDSERVICE_HPP */

