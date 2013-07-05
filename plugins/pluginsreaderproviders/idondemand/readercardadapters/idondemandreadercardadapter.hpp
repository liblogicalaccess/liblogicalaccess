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
	};

}

#endif /* LOGICALACCESS_DEFAULTIDONDEMANDREADERCARDADAPTER_HPP */

 
