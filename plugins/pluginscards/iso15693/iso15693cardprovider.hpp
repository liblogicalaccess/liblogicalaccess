/**
 * \file iso15693cardprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO15693 card provider.
 */

#ifndef LOGICALACCESS_ISO15693CARDPROVIDER_HPP
#define LOGICALACCESS_ISO15693CARDPROVIDER_HPP

#include "logicalaccess/cards/cardprovider.hpp"
#include "iso15693commands.hpp"


namespace logicalaccess
{
	class ISO15693Chip;

	/**
	 * \brief The ISO15693 card provider base class.
	 */
	class LIBLOGICALACCESS_API ISO15693CardProvider : public CardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ISO15693CardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO15693CardProvider();	

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
			 * \brief Write data on a specific Tag-It location, using given Tag-It keys.
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
			 * \brief Read data on a specific Tag-It location, using given Tag-It keys.
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

			/**
			 * \brief Get the associated ISO15693 chip.
			 * \return The ISO15693 chip.
			 */
			boost::shared_ptr<ISO15693Chip> getISO15693Chip();

			boost::shared_ptr<ISO15693Commands> getISO15693Commands() const { return boost::dynamic_pointer_cast<ISO15693Commands>(getCommands()); };

		protected:

	};
}

#endif
