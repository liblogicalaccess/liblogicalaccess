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

			std::vector<unsigned char> sendRplethCommand(const std::vector<unsigned char>& data, long timeout = 5000);

	};

}

#endif /* LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP */

 
