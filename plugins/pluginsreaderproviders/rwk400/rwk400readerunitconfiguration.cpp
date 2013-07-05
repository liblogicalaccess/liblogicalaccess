/**
 * \file rwk400readerunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 reader unit configuration.
 */

#include "rwk400readerunitconfiguration.hpp"
#include "rwk400readerprovider.hpp"

#ifdef UNIX
# include <termios.h>
# include <unistd.h>
#endif

namespace logicalaccess
{
	Rwk400ReaderUnitConfiguration::Rwk400ReaderUnitConfiguration()
		: ReaderUnitConfiguration(READER_RWK400)
	{
		resetConfiguration();
	}

	Rwk400ReaderUnitConfiguration::~Rwk400ReaderUnitConfiguration()
	{
	}

	void Rwk400ReaderUnitConfiguration::resetConfiguration()
	{
		d_baudrate = BRO_9600BAUD;
	}

	BaudRateOutput Rwk400ReaderUnitConfiguration::getBaudrate()
	{
		return d_baudrate;
	}

	void Rwk400ReaderUnitConfiguration::setBaudrate(BaudRateOutput baudrate)
	{
		d_baudrate = baudrate;
	}

	int Rwk400ReaderUnitConfiguration::getSystemBaudrate()
	{
		return getSystemBaudrateFromBaudrateOutput(d_baudrate);
	}

	void Rwk400ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("Baudrate", d_baudrate);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Rwk400ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_baudrate = static_cast<BaudRateOutput>(node.get_child("Baudrate").get_value<unsigned int>());
	}

	std::string Rwk400ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "Rwk400ReaderUnitConfiguration";
	}

	int Rwk400ReaderUnitConfiguration::getSystemBaudrateFromBaudrateOutput(BaudRateOutput baudrate)
	{
		int ret;

#ifndef _WINDOWS
		switch (baudrate)
		{
		case BRO_4800BAUD:
			ret = B4800;
			break;

		case BRO_9600BAUD:
			ret = B9600;
			break;

		case BRO_19200BAUD:
			ret = B19200;
			break;

		case BRO_38400BAUD:
			ret = B38400;
			break;

		case BRO_57600BAUD:
			ret = B57600;
			break;

		case BRO_115200BAUD:
			ret = B115200;
			break;

		default:
			ret = B115200;
			break;
		}
#else
		switch (baudrate)
		{
		case BRO_4800BAUD:
			ret = CBR_4800;
			break;

		case BRO_9600BAUD:
			ret = CBR_9600;
			break;

		case BRO_19200BAUD:
			ret = CBR_19200;
			break;

		case BRO_38400BAUD:
			ret = CBR_38400;
			break;

		case BRO_57600BAUD:
			ret = CBR_57600;
			break;

		case BRO_115200BAUD:
			ret = CBR_115200;
			break;

		default:
			ret = CBR_115200;
			break;
		}
#endif

		return ret;
	}
}

