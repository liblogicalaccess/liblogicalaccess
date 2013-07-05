/**
 * \file rwk400readerunit.hpp
 * \author Maxime C. <maxime-dppev@islog.com>
 * \brief Rwk400 Reader unit.
 */

#ifndef LOGICALACCESS_RWK400READERUNIT_HPP
#define LOGICALACCESS_RWK400READERUNIT_HPP

#include "rwk400readerunitconfiguration.hpp"
#include "logicalaccess/readerproviders/readerunit.hpp"

#define RWK400_MAFARE_DESFIRE_CHIP 0x08

namespace logicalaccess
{
	class Profile;
	class Rwk400ReaderCardAdapter;
	class Rwk400ReaderProvider;
	class ReaderCommunication;

	/**
	 * \brief The Rpleth reader unit class.
	 */
	class LIBLOGICALACCESS_API Rwk400ReaderUnit : public ReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			Rwk400ReaderUnit();

			/**
			 * \brief Destructor.
			 */
			virtual ~Rwk400ReaderUnit();

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
			 * \brief Get the reader communication for a card type.
			 * \param cardType The card type.
			 * \return The reader communication
			 */
			boost::shared_ptr<ReaderCommunication> getReaderCommunication(std::string cardType);

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
			 * \brief Get the current chip in air.
			 * \return The chip in air.
			 */
			virtual boost::shared_ptr<Chip> getChipInAir(unsigned int maxwait);

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
			 * \brief Get the reader ping command.
			 * \return The ping command.
			 */
			virtual std::vector<unsigned char> getPingCommand() const;

			/**
			 * \brief Get the card serial number.
			 * \return The card serial number.
			 */
			std::vector<unsigned char> getCardSerialNumber();

			/**
			 * \brief Get the default Rwk400 reader/card adapter.
			 * \return The default Rwk400 reader/card adapter.
			 */
			virtual boost::shared_ptr<Rwk400ReaderCardAdapter> getDefaultRwk400ReaderCardAdapter();			

			/**
			 * \brief Connect to the card.
			 * \return True if the card was connected without error, false otherwise.
			 *
			 * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
			 */
			bool connect();

			/**
			 * \brief Disconnect from the Rwk400.
			 * \see connect
			 *
			 * Calling this method on a disconnected Rwk400 has no effect.
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
			 * \brief Get the Rwk400 reader unit configuration.
			 * \return The Rwk400 reader unit configuration.
			 */
			boost::shared_ptr<Rwk400ReaderUnitConfiguration> getRwk400Configuration() { return boost::dynamic_pointer_cast<Rwk400ReaderUnitConfiguration>(getConfiguration()); };

			/**
			 * \brief Get the Rwk400 reader provider.
			 * \return The Rwk400 reader provider.
			 */
			boost::shared_ptr<Rwk400ReaderProvider> getRwk400ReaderProvider() const;

			/**
			 * \brief Select the card in the RFID Field.
			 * \return The card type (first byte) and his CSN.
			 */
			std::vector<unsigned char> selectCard();

			/**
			 * \brief Reset the RF field.
			 * \param offtime The offtime, in milliseconds. Default is 100.
			 */
			void resetRF(int offtime);

			/**
			 * \brief Send a RATS.
			 * \return true is the rats is successful or false otherwise.
			 */
			bool rats();

		protected:

			/**
			 * \brief The reader communication.
			 */
			boost::shared_ptr<ReaderCommunication> d_readerCommunication;
	};
}

#endif /* LOGICALACCESS_RWK400READERUNIT_HPP */
