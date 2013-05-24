/**
 * \file rplethreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Rpleth reader/card adapter. 
 */

#ifndef LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP

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
	class LIBLOGICALACCESS_API ISO7816RplethReaderCardAdapter : public RplethReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			ISO7816RplethReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816RplethReaderCardAdapter();

			/**
			 * \brief Send a command to the reader.
			 * \param command The command buffer.			 
			 * \param timeout The command timeout.
			 * \return The result of the command.
			 */
			std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long int timeout = 2000);
			
		protected:
			
	};

}

#endif /* LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP */

 

