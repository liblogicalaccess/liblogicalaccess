/**
 * \file PromagReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Promag reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../PromagReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace logicalaccess
{	
	/**
	 * \brief A default Promag reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API PromagReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			static const unsigned char STX; /**< \brief The start byte. */
			static const unsigned char ESC; /**< \brief The escape byte. */
			static const unsigned char BEL; /**< \brief The button pressed byte. */
			static const unsigned char CR; /**< \brief The first stop byte. */
			static const unsigned char LF; /**< \brief The second stop byte. */

			/**
			 * \brief Constructor.
			 */
			PromagReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~PromagReaderCardAdapter();
			
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
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);

			/**
			 * \brief Wait for a command.
			 * \param buf The buffer into which to put the received data.
			 * \param timeout The timeout value, in milliseconds. If timeout is negative, the call never times out.
			 * \return true if a command was received, false otherwise.
			 */
			bool receiveCommand(std::vector<unsigned char>& buf, long int timeout = 1000);
			
			/**
			 * \brief Get the Promag reader unit.
			 * \return The Promag reader unit.
			 */
			boost::shared_ptr<PromagReaderUnit> getPromagReaderUnit() const { return boost::dynamic_pointer_cast<PromagReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);
	};

}

#endif /* LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_H */

 
