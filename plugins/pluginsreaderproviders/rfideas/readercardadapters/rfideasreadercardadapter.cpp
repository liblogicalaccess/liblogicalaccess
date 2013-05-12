/**
 * \file rfideasreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RFIDeas reader/card adapter.
 */

#include "rfideasreadercardadapter.hpp"


namespace logicalaccess
{
	RFIDeasReaderCardAdapter::RFIDeasReaderCardAdapter()
		: ReaderCardAdapter()
	{
		
	}

	RFIDeasReaderCardAdapter::~RFIDeasReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> RFIDeasReaderCardAdapter::sendCommand(const std::vector<unsigned char>& /*command*/, long int /*timeout*/)
	{
		std::vector<unsigned char> r;

		return r;
	}
}
