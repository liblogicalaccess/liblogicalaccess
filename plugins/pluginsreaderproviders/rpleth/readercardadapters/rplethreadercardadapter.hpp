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
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);
			
			/**
			 * \brief Send a command in ascci to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendAsciiCommand(const std::string& command, long int timeout = 2000);

			/**
			 * \brief Receive the reader answer.
			 * \param command The sand command buffer.		 
			 * \param timeout The answer timeout.
			 * \return The answer.
			 */
			std::vector<unsigned char> receiveAnwser(const std::vector<unsigned char>& command, long int timeout = 2000);
			
			/**
			 * \brief Get the Rpleth reader unit.
			 * \return The Rpleth reader unit.
			 */
			boost::shared_ptr<RplethReaderUnit> getRplethReaderUnit() const { return boost::dynamic_pointer_cast<RplethReaderUnit>(getReaderUnit()); };			
			

		protected:

			/**
			 * \brief Calculate command checksum.
			 * \param data The data to calculate checksum
			 * \return The checksum.
			 */
			unsigned char calcChecksum(const std::vector<unsigned char>& data);
			
			/**
			 * \brief Handle a answer and give the associated data buffer.
			 * \param cmdbuf The answer buffer.
			 * \return The data buffer.
			 */
			std::vector<unsigned char> handleAnswerBuffer(const std::vector<unsigned char>& cmdbuf, const std::vector<unsigned char>& ansbuf);		
	};

}

#endif /* LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP */

 
