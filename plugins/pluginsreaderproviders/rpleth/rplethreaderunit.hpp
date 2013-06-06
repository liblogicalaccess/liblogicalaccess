/**
 * \file rplethreaderunit.hpp
 * \author Maxime C. <maxime-dppev@islog.com>
 * \brief Rpleth Reader unit.
 */

#ifndef LOGICALACCESS_RPLETHREADERUNIT_HPP
#define LOGICALACCESS_RPLETHREADERUNIT_HPP

#include "logicalaccess/readerproviders/readerunit.hpp"
#include "logicalaccess/readerproviders/serialportxml.hpp"
#include "rplethreaderunitconfiguration.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace logicalaccess
{
	class Profile;
	class RplethReaderCardAdapter;
	class RplethReaderProvider;

	typedef enum
	{
		RPLETH = 0x00,
		HID = 0X01,
		LCD = 0x02
	} Device;

	typedef enum
	{
		STATEDHCP = 0x01,
		DHCP = 0x02,
		MAC = 0x03,
		IP = 0x04,
		SUBNET = 0x05,
		GATEWAY = 0x06,
		PORT = 0x07,
		MESSAGE = 0x08,
		RESET = 0x09
	} RplethCommand;

	typedef enum
	{
		BEEP = 0x00,
		BLINKLED1 = 0x01,
		BLINKLED2 = 0x02,
		NOP = 0x03,
		BADGE = 0x04,
		COM = 0x05
	} HidCommand;

	typedef enum
	{
		DISPLAY = 0x00,
		DISPLAYT = 0X01,
		BLINK = 0X02,
		SCROLL = 0X03,
		DISPLAYTIME = 0x04
	} LcdCommand;

	typedef enum
	{
		MIFARE = '2',
		MIFAREULTRALIGHT = '5',
		DESFIRE = '6'
	} ChipType;

	/**
	 * \brief The Rpleth reader unit class.
	 */
	class LIBLOGICALACCESS_API RplethReaderUnit : public ReaderUnit
	{
		public:			

			/**
			 * \brief Constructor.
			 * \param ip The reader address
			 * \param port The port .
			 */
			RplethReaderUnit();

			/**
			 * \brief Destructor.
			 */
			virtual ~RplethReaderUnit();

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
			 * \brief Get the current chip in air.
			 * \return The chip in air.
			 */
			boost::shared_ptr<Chip> getChipInAir(unsigned int maxwait = 2000);

			/**
			 * \brief Get the default Elatec reader/card adapter.
			 * \return The default Elatec reader/card adapter.
			 */
			virtual boost::shared_ptr<RplethReaderCardAdapter> getDefaultRplethReaderCardAdapter();			

			/**
			 * \brief Connect to the card.
			 * \return True if the card was connected without error, false otherwise.
			 *
			 * If the card handle was already connected, connect() first call disconnect(). If you intend to do a reconnection, call reconnect() instead.
			 */
			bool connect();

			/**
			 * \brief Disconnect from the reader.
			 * \see connect
			 *
			 * Calling this method on a disconnected reader has no effect.
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
			 * \brief Get the Rpleth reader unit configuration.
			 * \return The Rpleth reader unit configuration.
			 */
			boost::shared_ptr<RplethReaderUnitConfiguration> getRplethConfiguration() { return boost::dynamic_pointer_cast<RplethReaderUnitConfiguration>(getConfiguration()); };

			/**
			 * \brief Get the Rpleth reader provider.
			 * \return The Rpleth reader provider.
			 */
			boost::shared_ptr<RplethReaderProvider> getRplethReaderProvider() const;

			/**
			 * \brief Get the client socket.
			 */
			boost::shared_ptr<boost::asio::ip::tcp::socket> getSocket();

			/**
			 * \brief Get reader dhcp state.
			 */
			bool getDhcpState();

			/**
			 * \brief Set the reader dhcp state.
			 */
			void setDhcpState(bool status);

			/**
			 * \brief Set the Rpleth IP Address.
			 * \param readerAddress The new Rpleth IP Address.
			 */
			void setReaderIp(std::vector<unsigned char> address);
			
			/**
			 * \brief Set the Rpleth MAC Address.
			 * \param readerAddress The new Rpleth MAC Address.
			 */
			void setReaderMac(std::vector<unsigned char> address);

			/**
			 * \brief Set the Rpleth Subnet mask.
			 * \param readerAddress The new Rpleth Subnet Subnet.
			 */
			void setReaderSubnet(std::vector<unsigned char> address);
			
			/**
			 * \brief Set the Rpleth Gateway Address.
			 * \param readerAddress The new Rpleth Gateway Address.
			 */
			void setReaderGateway(std::vector<unsigned char> address);

			/**
			 * \brief Set the Rpleth port.
			 * \param readerAddress The new Rpleth port.
			 */
			void setReaderPort(int port);

			/**
			 * \brief Set the Rpleth port.
			 * \param offset The offset in wiegand trame.
			 * \param length The length of code in wiegand.
			 * \param tramSize The size of wiegand trame.
			 */
			void setConfWiegand(unsigned char offset, unsigned char length, unsigned char tramSize);
			
			/**
			 * \brief Reset the Rpleth reader.
			 */
			void resetReader();

			/**
			 * \brief Set the Rpleth default message.
			 * \param message The new default message.
			 */
			void setReaderMessage(std::string message);

			/**
			 * \brief Send a nop.
			 */
			void nop();
			
			/**
			 * \brief Request the last badge.
			 * \param timeout The time to wait.
			 * \return The last badge.
			 */
			std::vector<unsigned char> badge(long int timeout = 2000);

			/**
			 * \brief Send a command.
			 * \param command The command to send.
			 * \return The answer.
			 */
			std::vector<unsigned char> sendCommandCom(std::string command);

			std::vector<unsigned char> AsciiToHex (std::vector<unsigned char> source);

		protected:
			
			/**
			 * \brief Client socket use to communicate with the reader.
			 */
			boost::shared_ptr<boost::asio::ip::tcp::socket> d_socket;

			/**
			 * \brief Provides core I/O functionality
			 */
			boost::asio::io_service ios;

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			std::vector<unsigned char> receiveBadge (long int timeout = 2000);

			/**
			 * \brief Take the csn from wiegand full trame.
			 * \param trame The wiegnad full trame.			 
			 * \return The csn contains into the wiegand trame.
			 */
			std::vector<unsigned char> getCsn (std::vector<unsigned char> trame);
	};
}

#endif
