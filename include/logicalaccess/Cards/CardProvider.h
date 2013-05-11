/**
 * \file CardProvider.h
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Card provider.
 */

#ifndef LOGICALACCESS_CARDPROVIDER_H
#define LOGICALACCESS_CARDPROVIDER_H

#include "logicalaccess/Services/CardService.h"
#include "logicalaccess/Cards/AccessInfo.h"
#include "logicalaccess/Cards/Location.h"
#include "logicalaccess/Cards/LocationNode.h"
#include "logicalaccess/Cards/Commands.h"


namespace logicalaccess
{
	class AccessInfo;	

	#define EXCEPTION_MSG_AUTHENTICATE		"Authenticate failed." /**< \brief The authentication exception message */
	#define EXCEPTION_MSG_LOADKEY			"Load key failed." /**< \brief The load key exception message */
	#define EXCEPTION_MSG_NOKEY				"No key found." /**< \brief The no key found exception message */
	#define EXCEPTION_MSG_READ				"Read operation failed." /**< \brief The read exception message */
	#define EXCEPTION_MSG_WRITE				"Write operation failed." /**< \brief The write exception message */
	#define EXCEPTION_MSG_CHANGEKEY			"Change key failed." /**< \brief The change key exception message */

	/**
	 * \brief The card behaviors.
	 */
	typedef enum {
		CB_DEFAULT = 0x0000, /**< Default behavior */
		CB_AUTOSWITCHAREA = 0x0001 /**< Auto switch area when needed (for read/write command) */
	} CardBehavior;

	/**
	 * \brief The base card provider class for all cards.
	 */
	class LIBLOGICALACCESS_API CardProvider : public boost::enable_shared_from_this<CardProvider>
	{
		public:

			/**
			 * \brief Constructor.
			 */
			CardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~CardProvider();

			/**
			 * \brief Erase the card.
			 * \return true if the card was erased, false otherwise.
			 */
			virtual bool erase() = 0;			

			/**
			 * \brief Erase a specific location on the card.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use to delete.
			 * \return true if the card was erased, false otherwise.
			 */
			virtual bool erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse) = 0;			

			/**
			 * \brief Write data on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for write access.
			 * \param aiToWrite The key's informations to change.
			 * \param data Data to write.
			 * \param dataLength Data's length to write.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual	bool writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior behaviorFlags) = 0;

			/**
			 * \brief Read data on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use for read access.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \param behaviorFlags Flags which determines the behavior.
			 * \return True on success, false otherwise.
			 */
			virtual bool readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags) = 0;

			/**
			 * \brief Read data header on a specific location, using given keys.
			 * \param location The data location.
			 * \param aiToUse The key's informations to use.
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length to read.
			 * \return Data header length.
			 */
			virtual size_t readDataHeader(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength) = 0;

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

			/*
			 * \brief Get the associated chip object.
			 * \return The chip.
			 */
			boost::shared_ptr<Chip> getChip() { return d_chip.lock(); };

			/*
			 * \brief Set the associated chip object.
			 * \param chip The chip.
			 */
			void setChip(boost::weak_ptr<Chip> chip) { d_chip = chip; };

			/*
			 * \brief Get the associated commands object.
			 * \return The commands.
			 */
			boost::shared_ptr<Commands> getCommands() const { return d_commands; };

			/*
			 * \brief Set the associated commands object.
			 * \param commands The commands.
			 */
			void setCommands(boost::shared_ptr<Commands> commands) { d_commands = commands; };
			

		protected:

			/**
			 * \brief Chip object.
			 */
			boost::weak_ptr<Chip> d_chip;

			/**
			 * \brief Commands object.
			 */
			boost::shared_ptr<Commands> d_commands;
	};
}

#endif
