/**
 * \file STidSTRReaderUnit.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR Reader unit.
 */

#ifndef LOGICALACCESS_STIDSTRREADERUNIT_H
#define LOGICALACCESS_STIDSTRREADERUNIT_H

#include "logicalaccess/ReaderProviders/ReaderUnit.h"
#include "logicalaccess/ReaderProviders/SerialPortXml.h"
#include "STidSTRReaderUnitConfiguration.h"

namespace logicalaccess
{
	class Profile;
	class STidSTRReaderCardAdapter;
	class STidSTRReaderProvider;

	/**
	 * \brief The STid baudrates.
	 */
	typedef enum {
		STID_BR_9600 = 0x00, /**< 9600 baudrate */
		STID_BR_19200 = 0x01, /**< 19200 baudrate */
		STID_BR_38400 = 0x02, /**< 38400 baudrate */
		STID_BR_57600 = 0x03, /**< 57600 baudrate */
		STID_BR_115200 = 0x04	 /**< 115200 baudrate */
	} STidBaudrate;

	/**
	 * \brief The STid tamper switch behavior.
	 */
	typedef enum {
		STID_TS_NOTHING = 0x00, /**< Do nother when pullout */
		STID_TS_RESET_CONFIG = 0x01, /**< Reset all configuration (baudrate, rs485 address, session keys, ...) */
		STID_TS_RESET_KEYS = 0x02, /**< Reset user keys */
		STID_TS_RESET_ALL = 0x03 /**< Reset all configuration and user keys */
	} STidTamperSwitchBehavior;

	/**
	 * \brief The STidSTR reader unit class.
	 */
	class LIBLOGICALACCESS_API STidSTRReaderUnit : public ReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			STidSTRReaderUnit(boost::shared_ptr<SerialPortXml> port);

			/**
			 * \brief Destructor.
			 */
			virtual ~STidSTRReaderUnit();

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
			 * \brief Get the serial port information.
			 * \return The serial port.
			 */
			virtual boost::shared_ptr<SerialPortXml> getSerialPort();

			/**
			 * \brief Set the serial port information.
			 * \param port The serial port.
			 */
			virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port);

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
			 * \brief Get the default STidSTR reader/card adapter.
			 * \return The default STidSTR reader/card adapter.
			 */
			virtual boost::shared_ptr<STidSTRReaderCardAdapter> getDefaultSTidSTRReaderCardAdapter();			

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
			 * \brief Get the STidSTR reader unit configuration.
			 * \return The STidSTR reader unit configuration.
			 */
			boost::shared_ptr<STidSTRReaderUnitConfiguration> getSTidSTRConfiguration() { return boost::dynamic_pointer_cast<STidSTRReaderUnitConfiguration>(getConfiguration()); };

			/**
			 * \brief Get the STidSTR reader provider.
			 * \return The STidSTR reader provider.
			 */
			boost::shared_ptr<STidSTRReaderProvider> getSTidSTRReaderProvider() const;

			/**
			 * \brief Configure the current serial port.
			 */
			void configure();

			/**
			 * \brief Configure the given serial port.
			 * \param port The COM port to configure.
			 * \param retryConfiguring On error, the function wait some milliseconds and retry another time to configure the serial port.
			 */
			void configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring);

			/**
			 * \brief Start to auto-detect the first serial port with a reader. Update serial port when found.
			 */
			void startAutoDetect();

			/**
			 * \brief Scan the RFID field for ISO 14443-A tag detection.
			 * \return The chip object if a tag is inserted.
			 */
			boost::shared_ptr<Chip> scan14443A();

			/**
			 * \brief Scan the RFID field for ISO 14443-A tag detection.
			 * \return The chip object if a tag is inserted.
			 */
			boost::shared_ptr<Chip> scanARaw();

			/**
			 * \brief Scan the RFID field for ISO 14443-B tag detection.
			 * \return The chip object if a tag is inserted.
			 */
			boost::shared_ptr<Chip> scan14443B();

			/**
			 * \brief Authenticate the host with the reader and genereate session keys for HMAC and enciphering, 1/2.
			 * \param isHMAC True if the authentication process is for HMAC session key, false for AES session key.
			 * \return The reader response.
			 */
			std::vector<unsigned char> authenticateReader1(bool isHMAC);

			/**
			 * \brief Authenticate the host with the reader and genereate session keys for HMAC and enciphering, 2/2.
			 * \param data The authentication command data.
			 * \return The reader response.
			 */
			std::vector<unsigned char> authenticateReader2(const std::vector<unsigned char>& data);

			/**
			 * \brief Reset the reader authentication and session keys. Next frames will be transmitted plain.
			 */
			void ResetAuthenticate();

			/**
			 * \brief Change keys on the reader memory.
			 * \param key_hmac The new HMAC user key. Empty to keep the current one.
			 * \param key_hmac The new AES user key. Empty to keep the current one.
			 */
			void ChangeReaderKeys(const std::vector<unsigned char>& key_hmac, const std::vector<unsigned char>& key_aes);

			/**
			 * \brief Set the reader baudrate for serial communication.
			 * \param baudrate The new baudrate to apply.
			 */
			void setBaudRate(STidBaudrate baudrate);

			/**
			 * \brief Set the RS485 reader address.
			 * \param address The new reader address.
			 */
			void set485Address(unsigned char address);

			/**
			 * \brief STidSTR reader information.
			 */
			struct STidSTRInformation
			{
				/**
				 * \brief The reader version;
				 */
				unsigned char version;

				/**
				 * \brief The reader baudrate use for serial communication.
				 */
				STidBaudrate baudrate;

				/**
				 * \brief The RS485 reader address.
				 */
				unsigned char rs485Address;

				/**
				 * \brief The power voltage actually used on the reader.
				 */
				float voltage;
			};

			/**
			 * \brief Get the reader information.
			 * \return The reader information.
			 */
			STidSTRInformation getReaderInformaton();

			/**
			 * \brief Set the allowed communication modes with the reader.
			 * \param plainComm True to be allowed to use plain communication mode, false otherwise.
			 * \param signedComm True to be allowed to use signed communication mode without ciphering, false otherwise.
			 * \param cipheredComm True to be allowed to use ciphered communication mode without signature, false otherwise.
			 */
			void setAllowedCommModes(bool plainComm, bool signedComm, bool cipheredComm);

			/**
			 * \brief Set the tamper switch settings.
			 * \param useTamperSwitch True to use tamper switch action, false to not.
			 * \param behavior The behavior when tamper switch is activated.
			 */
			void setTamperSwitchSettings(bool useTamperSwitch, STidTamperSwitchBehavior behavior);

			/**
			 * \brief Get the tamper switch settings.
			 * \param useTamperSwitch True to use tamper switch action, false to not.
			 * \param behavior The behavior when tamper switch is activated.
			 * \param swChanged True if the SW state have changed, false otherwise.
			 */
			void getTamperSwitchInfos(bool& useTamperSwitch, STidTamperSwitchBehavior& behavior, bool& swChanged);

			/**
			 * \brief Save in EEPROM INDEX table memory the keys from a SKB card.
			 */
			void loadSKB();

			/**
			 * \brief The the HMAC session key.
			 * \return The session key.
			 */
			std::vector<unsigned char> getSessionKeyHMAC() const { return d_sessionKey_hmac; };

			/**
			 * \brief The the AES session key.
			 * \return The session key.
			 */
			std::vector<unsigned char> getSessionKeyAES() const { return d_sessionKey_aes; };

		protected:

			/**
			 * \brief The auto-detected status
			 */
			bool d_isAutoDetected;

			/**
			 * \brief Authenticate the host and the reader to obtain the HMAC session key.
			 */
			void authenticateHMAC();

			/**
			 * \brief Authenticate the host and the reader to obtain the AES session key.
			 */
			void authenticateAES();

			/**
			 * \brief The serial port.
			 */
			boost::shared_ptr<SerialPortXml> d_port;

			/**
			 * \brief The HMAC session key.
			 */
			std::vector<unsigned char> d_sessionKey_hmac;

			/**
			 * \brief The AES session key.
			 */
			std::vector<unsigned char> d_sessionKey_aes;
	};
}

#endif
