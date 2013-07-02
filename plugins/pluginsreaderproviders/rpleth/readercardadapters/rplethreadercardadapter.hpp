/**
 * \file rplethreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Rpleth reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "../rplethreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{	
	/**
	 * \brief A default Rpleth reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API RplethReaderCardAdapter : public ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			RplethReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~RplethReaderCardAdapter();

			/**
			 * \brief Adapt the command to send to the reader.
			 * \param command The command to send.
			 * \return The adapted command to send.
			 */
			virtual std::vector<unsigned char> adaptCommand(const std::vector<unsigned char>& command);

			/**
			 * \brief Adapt the asnwer received from the reader.
			 * \param answer The answer received.
			 * \return The adapted answer received.
			 */
			virtual std::vector<unsigned char> adaptAnswer(const std::vector<unsigned char>& answer);
			
			/**
			 * \brief Send a command in ascci to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendAsciiCommand(const std::string& command, long int timeout = 2000);
			

		protected:

			/**
			 * \brief Calculate command checksum.
			 * \param data The data to calculate checksum
			 * \return The checksum.
			 */
			unsigned char calcChecksum(const std::vector<unsigned char>& data);	
	};

}

#endif /* LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP */

 
