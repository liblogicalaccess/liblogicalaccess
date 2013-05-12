/**
 * \file stidstrreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default STidSTR reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "../stidstrreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{	
	/**
	 * \brief The STid command type.
	 */
	typedef enum {
		STID_CMD_READER = 0x00, /**< Reader command type */
		STID_CMD_MIFARE_CLASSIC = 0x01, /**< Mifare classic command type */
		STID_CMD_DESFIRE = 0x02, /**< DESFire command type */
		STID_CMD_MIFARE_PLUS = 0x03, /**< Mifare plus command type */
		STID_CMD_RESERVED = 0x04, /**< Reserved command type */
		STID_CMD_MIFARE_ULTRALIGHTC = 0x05, /**< Mifare ultralight C command type */
		STID_CMD_RFIDCHIP = 0x06 /**< RFID chip command type */
	} STidCmdType;

	/**
	 * \brief A default STidSTR reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API STidSTRReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 * \param adapterType The adapter type.
			 */
			STidSTRReaderCardAdapter(STidCmdType adapterType = STID_CMD_READER);

			/**
			 * \brief Destructor.
			 */
			virtual ~STidSTRReaderCardAdapter();

			static const unsigned char SOF; /**< \brief The start of message value. */

			/**
			 * \brief Send a command to the reader without waiting for a response.
			 * \param commandCode The command code.
			 * \param command The command buffer.
			 * \return True if the command was send successfully, false otherwise.
			 */
			bool sendCommandWithoutResponse(unsigned short commandCode, const std::vector<unsigned char>& command);

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer, including command code.
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

			/**
			 * \brief Send a command to the reader.
			 * \param commandCode The command code.
			 * \param command The command buffer.
			 * \param statusCode Will contains the response status code.
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(unsigned short commandCode, const std::vector<unsigned char>& command, unsigned char& statusCode, long int timeout = 2000);

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param statusCode Will contains the response status code.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, unsigned char& statusCode, long int timeout = 2000);
			
			/**
			 * \brief Get the Deister reader unit.
			 * \return The Deister reader unit.
			 */
			boost::shared_ptr<STidSTRReaderUnit> getSTidSTRReaderUnit() const { return boost::dynamic_pointer_cast<STidSTRReaderUnit>(getReaderUnit()); };			

			/**
			 * \brief Calculate the message HMAC.
			 * \param buf The message buffer.
			 * \return The HMAC.
			 */
			std::vector<unsigned char> calculateHMAC(const std::vector<unsigned char>& buf) const;
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \param statusCode Will contains the response status code.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf, unsigned char& statusCode);

			/**
			 * \brief Process message data to send.
			 * \param commandCode The command code.
			 * \param command The command message data.
			 * \return The processed message (signed/ciphered).
			 */
			std::vector<unsigned char> sendMessage(unsigned short commandCode, const std::vector<unsigned char>& command);

			/**
			 * \brief Process message response to return plain message data and status code.
			 * \param data The raw data from reader.
			 * \param statusCode Will contains the response status code.
			 * \return The plain message data.
			 */
			std::vector<unsigned char> receiveMessage(const std::vector<unsigned char>& data, unsigned char& statusCode);

			/**
			 * \brief Check status code and throw exception on error.
			 * \param statusCode The status code.
			 */
			void CheckError(unsigned char statusCode) const;


			/**
			 * \brief Get the initialization vector for encryption.
			 * \return The IV.
			 * \remarks This behavior is a security threat into STid communication protocol !
			 */
			std::vector<unsigned char> getIV();

			/**
			 * \brief The adapter type.
			 */
			STidCmdType d_adapterType;

			/**
			 * \brief The last command code sent.
			 */
			unsigned short d_lastCommandCode;

			/**
			 * \brief The last IV to use.
			 */
			std::vector<unsigned char> d_lastIV;
	};

}

#endif /* LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_H */

 
