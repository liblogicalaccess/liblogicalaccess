/**
 * \file DESFireCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire card provider.
 */

#ifndef LOGICALACCESS_DESFIRECARDPROVIDER_H
#define LOGICALACCESS_DESFIRECARDPROVIDER_H

#include "logicalaccess/Cards/CardProvider.h"
#include "DESFireCommands.h"


namespace logicalaccess
{
	class DESFireChip;


	#define EXCEPTION_MSG_SELECTAPPLICATION			"Select application failed." /**< \brief The select application exception message */

	/**
	 * \brief The DESFire base card provider class.
	 */
	class LIBLOGICALACCESS_API DESFireCardProvider : public CardProvider
	{
		public:

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
			 * \brief Select an application.
			 * \param location The DESFire location
			 */
			virtual void selectApplication(boost::shared_ptr<DESFireLocation> location);

			/**
			 * \brief Create a new application.
			 * \param location The DESFire location
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, int maxNbKeys);

			/**
			 * \brief Create a new data file in the current application.
			 * \param location The DESFire location
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 */
			virtual void createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, int fileSize);

			/**
			 * \brief Write data on a specific DESFire location, using given DESFire keys.
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
			 * \brief Read data on a specific DESFire location, using given DESFire keys.
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
			 * \brief Get the communication mode for a file.
			 * \param aiToUse DESFire access information to use
			 * \param fileno The file number
		     * \param isReadMode Is read or write mode
			 * \param needLoadKey Set if it's necessary to be authenticate for the access.
			 * \return The communication mode.
			 */
			virtual EncryptionMode getEncryptionMode(boost::shared_ptr<AccessInfo> aiToUse, unsigned char fileno, bool isReadMode, bool* needLoadKey);

			/**
			 * \brief Get the length of a file.
			 * \param fileno The file number
			 * \return The length.
			 */
			virtual size_t getFileLength(unsigned char fileno);

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

			/*
			 * \brief Get the associated DESFire chip.
			 * \return The DESFire chip.
			 */
			boost::shared_ptr<DESFireChip> getDESFireChip();

			boost::shared_ptr<DESFireCommands> getDESFireCommands() const { return boost::dynamic_pointer_cast<DESFireCommands>(getCommands()); };

		protected:

			
	};
}

#endif /* LOGICALACCESS_DESFIRECARDPROVIDER_H */

