/**
 * \file rplethdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth data transport.
 */

#include "rplethdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{
	RplethDataTransport::RplethDataTransport()
		: TcpDataTransport()
	{
		
	}

	RplethDataTransport::~RplethDataTransport()
	{

	}

	void RplethDataTransport::send(const std::vector<unsigned char>& data)
	{
		
	}

	std::vector<unsigned char> RplethDataTransport::receive(long int timeout)
	{
		
	}

	std::string RplethDataTransport::getDefaultXmlNodeName() const
	{
		return "RplethDataTransport";
	}
}