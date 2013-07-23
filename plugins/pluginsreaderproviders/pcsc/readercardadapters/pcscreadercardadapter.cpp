/**
 * \file pcscreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader/card adapter.
 */

#include "pcscreadercardadapter.hpp"
#include "../pcscreaderunit.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "../pcscdatatransport.hpp"


namespace logicalaccess
{
	PCSCReaderCardAdapter::PCSCReaderCardAdapter()
	{
		d_dataTransport.reset(new PCSCDataTransport());
	}

	PCSCReaderCardAdapter::~PCSCReaderCardAdapter()
	{

	}	

	std::vector<unsigned char> PCSCReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
	{
		return command;
	}

	std::vector<unsigned char> PCSCReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
	{
		return answer;
	}
}
