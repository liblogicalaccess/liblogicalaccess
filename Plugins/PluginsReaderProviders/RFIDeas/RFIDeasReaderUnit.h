/**
 * \file RFIDeasReaderUnit.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief RFIDeas Reader unit.
 */

#ifndef LOGICALACCESS_RFIDEASREADERUNIT_H
#define LOGICALACCESS_RFIDEASREADERUNIT_H

#include "logicalaccess/ReaderProviders/ReaderUnit.h"
#include "RFIDeasReaderUnitConfiguration.h"

#ifdef _WINDOWS
#include <rfideas/pcProxAPI.h>
#endif

namespace LOGICALACCESS
{
	class Profile;
	class RFIDeasReaderCardAdapter;
	class RFIDeasReaderProvider;	

	typedef void (*setTagIdBitsLengthFct)(boost::shared_ptr<LOGICALACCESS::Chip>*, unsigned int);

	/**
	 * \brief The RFIDeas reader unit class.
	 */
	class LIBLOGICALACCESS_API RFIDeasReaderUnit : public ReaderUnit
	{
		public:			

			/**
			 * \brief Destructor.
			 */
			virtual ~RFIDeasReaderUnit();

			/**
			 * \brief Get the reader unit name.
			 * \return The reader unit name.
			 */
			virtual std::string getName() const;

			/**
			 * \brief Get the connected reader unit name.
			 * \return The connected reader unit name.
			 */
			virtual std::string getConnectedName();			

			/**
			 * \brief Set the card type.
			 * \param cardType The card type.
			 */
			virtual void setCardType(std::string cardType);			

			/**
			 * \brief Wait for a card insertion.
			 * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
			 * \return True if a card was inserted, false otherwise. If a card was inserted, the name of the reader on which the insertion was detected is accessible with getReader().
			 * \warning If the card is already connected, then the method always fail.
			 */
			virtual bool waitInsertion(unsigned int maxwait);
	
			/**
			 * \brief Wait for a card removal.
			 * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero, then the call never times out.
			 * \return True if a card was removed, false otherwise. If a card was removed, the name of the reader on which the removal was detected is accessible with getReader().
			 */
			virtual bool waitRemoval(unsigned int maxwait);

			/**
			 * \brief Create the chip object from card type.
			 * \param type The card type.
			 * \return The chip.
			 */
			virtual boost::shared_ptr<Chip> createChip(std::string type);

			/**
			 * \brief Get the first and/or most accurate chip found.
			 * \return The single chip.
			 */
			virtual boost::shared_ptr<Chip> getSingleChip();

			/**
			 * \brief Get chip available in the RFID rang.
			 * \return The chip list.
			 */
			virtual std::vector<boost::shared_ptr<Chip> > getChipList();

			/**
			 * \brief Get the tag id.
			 * \return The tag id.
			 */
			std::vector<unsigned char> getTagId();

			/**
			 * \brief Get the default RFIDeas reader/card adapter.
			 * \return The default RFIDeas reader/card adapter.
			 */
			virtual boost::shared_ptr<RFIDeasReaderCardAdapter> getDefaultRFIDeasReaderCardAdapter();			

			/**
			 * \brief Connect to the card.
			 * \return True if the card was connected without error, false otherwise.
			 *
			 * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
			 */
			bool connect();

			/**
			 * \brief Disconnect from the Deister.
			 * \see connect
			 *
			 * Calling this method on a disconnected Deister has no effect.
			 */
			void disconnect();

			/**
			 * \brief Check if the card is connected.
			 * \return True if the card is connected, false otherwise.
			 */
			virtual bool isConnected();

			/**
			 * \brief Connect to the reader. Implicit connection on first command sent.
			 * \return True if the connection successed.
			 */
			virtual bool connectToReader();

			/**
			 * \brief Disconnect from reader.
			 */
			virtual void disconnectFromReader();

			/**
			 * \brief Get a string hexadecimal representation of the reader serial number
			 * \return The reader serial number or an empty string on error.
			 */
			virtual std::string getReaderSerialNumber();

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node);

			/**
			 * \brief Get the Deister reader unit configuration.
			 * \return The Deister reader unit configuration.
			 */
			boost::shared_ptr<RFIDeasReaderUnitConfiguration> getRFIDeasConfiguration() { return boost::dynamic_pointer_cast<RFIDeasReaderUnitConfiguration>(getConfiguration()); };

			/**
			 * \brief Get the Deister reader provider.
			 * \return The Deister reader provider.
			 */
			boost::shared_ptr<RFIDeasReaderProvider> getRFIDeasReaderProvider() const;

			/**
			 * \brief Get the reader unit singleton instance.
			 * \return The reader unit instance.
			 */
			static boost::shared_ptr<RFIDeasReaderUnit> getSingletonInstance();

		protected:

			/**
			 * \brief Constructor.
			 */
			RFIDeasReaderUnit();

			/**
			 * \brief Initialize external functions.
			 */
			void initExternFct();

			/**
			 * \brief Uninitialize external functions.
			 */
			void uninitExternFct();

			/**
			 * \brief Initialize reader connection (USB or Serial port).
			 */
			void initReaderCnx();

			/**
			 * \brief Uninitialize reader connection.
			 */
			void uninitReaderCnx();		


#ifdef _WINDOWS
			GetLibVersion fnGetLibVersion;
			USBConnect fnUSBConnect;
			ComConnect fnCOMConnect;
			ComDisconnect fnCOMDisconnect;
			SetComSrchRange fnSetComSrchRange;
			ReadCfg fnReadCfg;
			GetFlags fnGetFlags;
			SetFlags fnSetFlags;
			WriteCfg fnWriteCfg;
			GetLastLibErr fnGetLastLibErr;
			GetActiveID fnGetActiveID;
			GetActiveID32 fnGetActiveID32;
			USBDisconnect fnUSBDisconnect;
			GetDevCnt fnGetDevCnt;
			SetActDev fnSetActDev;
			SetLUID fnSetLUID;
			GetQueuedID fnGetQueuedID;
			GetQueuedID_index fnGetQueuedID_index;

			HMODULE hWejAPIDLL;
#endif

			long d_deviceId;
			bool isCOMConnection;

			unsigned int d_lastTagIdBitsLength;
	};
}

#endif
