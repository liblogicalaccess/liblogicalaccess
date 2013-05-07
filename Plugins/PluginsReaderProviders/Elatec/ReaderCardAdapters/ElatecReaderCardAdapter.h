/**
 * \file PromagReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Elatec reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../ElatecReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace LOGICALACCESS
{	
	/**
	 * \brief A default Elatec reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API ElatecReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			ElatecReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~ElatecReaderCardAdapter();
			
			/**
			 * \brief Send a command to the reader without waiting for a response.	
			 * \param cmdcode The command code.
			 * \param command The command buffer.
			 * \return True if the command was send successfully, false otherwise.
			 */
			bool sendCommandWithoutResponse(unsigned char cmdcode, const std::vector<unsigned char>& command);

			/**
			 * \brief Send a command to the reader.
			 * \param cmdcode The command code.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(unsigned char cmdcode, const std::vector<unsigned char>& command, long int timeout = 2000);

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
			 * \brief Get the Elatec reader unit.
			 * \return The Elatec reader unit.
			 */
			boost::shared_ptr<ElatecReaderUnit> getElatecReaderUnit() const { return boost::dynamic_pointer_cast<ElatecReaderUnit>(getReaderUnit()); };			
			

		protected:

			/**
			 * \brief Calculate the buffer checksum.
			 * \param buf The buffer.
			 * \return The checksum value.
			 */
			unsigned char calcChecksum(const std::vector<unsigned char>& buf) const;
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);

			/**
			 * \brief The last command code used.
			 */
			unsigned char d_last_cmdcode;
	};

}

#endif /* LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_H */

 
