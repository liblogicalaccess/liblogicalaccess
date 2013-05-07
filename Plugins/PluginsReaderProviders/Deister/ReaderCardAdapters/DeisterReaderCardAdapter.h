/**
 * \file DeisterReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Deister reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../DeisterReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A default Deister reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API DeisterReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			DeisterReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~DeisterReaderCardAdapter();

			static const unsigned char SHFT; /**< \brief The shift value. */
			static const unsigned char SOM; /**< \brief The start of message value. */
			static const unsigned char SOC; /**< \brief The start of command value. */
			static const unsigned char STOP; /**< \brief The stop value, end of message. */
			static const unsigned char Dummy; /**< \brief The dummy value. */
			static const unsigned char SHFT_SHFT; /**< \brief The shift 00 value to replace SHFT value in message. */
			static const unsigned char SHFT_SOM; /**< \brief The shift 00 value to replace SOM value in message. */
			static const unsigned char SHFT_SOC; /**< \brief The shift 00 value to replace SOC value in message. */
			static const unsigned char SHFT_STOP; /**< \brief The shift 00 value to replace STOP value in message. */
			
			/**
			 * \brief Send a command to the reader without waiting for a response.
			 * \param command The command buffer.
			 * \return True if the command was send successfully, false otherwise.
			 */
			bool sendCommandWithoutResponse(const std::vector<unsigned char>& command);

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 3000);

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, long int timeout = 2000);
			
			/**
			 * \brief Get the Deister reader unit.
			 * \return The Deister reader unit.
			 */
			boost::shared_ptr<DeisterReaderUnit> getDeisterReaderUnit() const { return boost::dynamic_pointer_cast<DeisterReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);

			/**
			 * \brief Prepare data buffer for device.
			 * \param data The data buffer to prepare.
			 * \return The data buffer ready to send to device.
			 */
			std::vector<unsigned char> prepareDataForDevice(const std::vector<unsigned char>& data) const;

			/**
			 * \brief Prepare data buffer from device.
			 * \param data The data buffer to prepare.
			 * \return The data buffer ready to use.
			 */
			std::vector<unsigned char> prepareDataFromDevice(const std::vector<unsigned char>& data) const;

			/**
			 * \brief Target device bus address destination.
			 */
			unsigned char d_destination;

			/**
			 * \brief Transmitter bus address source.
			 */
			unsigned char d_source;
	};

}

#endif /* LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_H */

 
