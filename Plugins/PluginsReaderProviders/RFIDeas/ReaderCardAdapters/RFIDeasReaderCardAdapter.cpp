/**
 * \file RFIDeasReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief RFIDeas reader/card adapter.
 */

#include "RFIDeasReaderCardAdapter.h"


namespace LOGICALACCESS
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
