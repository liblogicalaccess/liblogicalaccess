
/**	
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "iso7816rplethreadercardadapter.hpp"

namespace logicalaccess
{		
	ISO7816RplethReaderCardAdapter::ISO7816RplethReaderCardAdapter()
		: RplethReaderCardAdapter()
	{
		
	}

	ISO7816RplethReaderCardAdapter::~ISO7816RplethReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::sendCommand(std::vector<unsigned char>& command, long int timeout)
	{
		command[2] ++;
		command.emplace (command.begin()+3, 't');
		return RplethReaderCardAdapter::sendCommand (command, timeout);
	}
}
