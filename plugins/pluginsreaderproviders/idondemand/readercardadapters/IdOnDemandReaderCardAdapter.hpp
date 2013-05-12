/**
 * \file idondemandreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default IdOnDemand reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTIDONDEMANDREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTIDONDEMANDREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "../idondemandreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{	
	/**
	 * \brief A default IdOnDemand reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API IdOnDemandReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			IdOnDemandReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~IdOnDemandReaderCardAdapter();

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
			bool receiveCommand(std::vector<unsigned char>& buf, long int timeout = 2000);
			
			/**
			 * \brief Get the Elatec reader unit.
			 * \return The Elatec reader unit.
			 */
			boost::shared_ptr<IdOnDemandReaderUnit> getIdOnDemandReaderUnit() const { return boost::dynamic_pointer_cast<IdOnDemandReaderUnit>(getReaderUnit()); };			
			

		protected:
			
			/**
			 * \brief Handle a command buffer and give the associated data buffer.
			 * \param cmdbuf The command buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleCommandBuffer(const std::vector<unsigned char>& cmdbuf);
	};

}

#endif /* LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_HPP */

 
