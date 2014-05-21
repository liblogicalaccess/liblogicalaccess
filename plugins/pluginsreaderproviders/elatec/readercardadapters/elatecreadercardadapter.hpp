/**
 * \file promagreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Elatec reader/card adapter. 
 */

#ifndef LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "../elatecreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"


namespace logicalaccess
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
			 * \brief Send a command to the reader.
			 * \param cmdcode The command code.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			virtual std::vector<unsigned char> sendCommand(unsigned char cmdcode, const std::vector<unsigned char>& command, long int timeout = 2000);

		protected:

			/**
			 * \brief Calculate the buffer checksum.
			 * \param buf The buffer.
			 * \return The checksum value.
			 */
			static unsigned char calcChecksum(const std::vector<unsigned char>& buf);

			/**
			 * \brief The last command code used.
			 */
			unsigned char d_last_cmdcode;
	};

}

#endif /* LOGICALACCESS_DEFAULTELATECREADERCARDADAPTER_H */

 
