/**
 * \file AxessTMCLegicReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default AxessTMCLegic reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTAXESSTMCLEGICREADERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTAXESSTMCLEGICREADERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../AxessTMCLegicReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace logicalaccess
{	
	/**
	 * \brief A default AxessTMCLegic reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API AxessTMCLegicReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			AxessTMCLegicReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~AxessTMCLegicReaderCardAdapter();

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
			 * \brief Get the Deister reader unit.
			 * \return The Deister reader unit.
			 */
			boost::shared_ptr<AxessTMCLegicReaderUnit> getAxessTMCLegicReaderUnit() const { return boost::dynamic_pointer_cast<AxessTMCLegicReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);
	};

}

#endif /* LOGICALACCESS_DEFAULTAXESSTMCLEGICREADERCARDADAPTER_H */

 
