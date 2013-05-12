/**
 * \file AxessTMC13ReaderCardAdapter.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default AxessTMC13 reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_H
#define LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_H

#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "../AxessTMC13ReaderUnit.h"

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logicalaccess/logs.h"

namespace logicalaccess
{	
	/**
	 * \brief A default AxessTMC13 reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API AxessTMC13ReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			static const unsigned char START0; /**< \brief The first start byte. */
			static const unsigned char START1; /**< \brief The second start byte. */
			static const unsigned char CR; /**< \brief The stop byte. */

			/**
			 * \brief Constructor.
			 */
			AxessTMC13ReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~AxessTMC13ReaderCardAdapter();
			
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
			boost::shared_ptr<AxessTMC13ReaderUnit> getAxessTMC13ReaderUnit() const { return boost::dynamic_pointer_cast<AxessTMC13ReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);
	};

}

#endif /* LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_H */

 
