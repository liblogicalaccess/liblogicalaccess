/**
 * \file SCIELReaderUnitConfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader unit configuration.
 */

#include "SCIELReaderUnitConfiguration.h"

#ifdef UNIX
# include <termios.h>
# include <unistd.h>
#endif

namespace logicalaccess
{
	SCIELReaderUnitConfiguration::SCIELReaderUnitConfiguration()
		: ReaderUnitConfiguration("SCIEL")
	{
		resetConfiguration();
	}

	SCIELReaderUnitConfiguration::~SCIELReaderUnitConfiguration()
	{
	}

	void SCIELReaderUnitConfiguration::resetConfiguration()
	{
		d_tagOutThreshold = 150;
		d_tagInThreshold = 140;
		d_safetyThreshold = 120;
		d_timeBeforeTagIn = 1000;
		d_timeBeforeTagOut = 2000;
		d_timeBeforeSafety = 2000;
		d_timeRemoval = 2;
#ifndef UNIX
		d_portBaudRate = CBR_9600;
#else
		d_portBaudRate = B9600;
#endif
	}

	void SCIELReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("TagOutThreshold", d_tagOutThreshold);
		node.put("TagInThreshold", d_tagInThreshold);
		node.put("SafetyThreshold", d_safetyThreshold);
		node.put("TimeBeforeTagIn", d_timeBeforeTagIn);
		node.put("TimeBeforeTagOut", d_timeBeforeTagOut);
		node.put("TimeBeforeSafety", d_timeBeforeSafety);
		node.put("TimeRemoval", d_timeRemoval);
		node.put("PortBaudRate", d_portBaudRate);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SCIELReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& node)
	{
		d_tagOutThreshold = node.get_child("TagOutThreshold").get_value<unsigned char>();
		d_tagInThreshold = node.get_child("TagInThreshold").get_value<unsigned char>();
		d_safetyThreshold = node.get_child("SafetyThreshold").get_value<unsigned char>();
		d_timeBeforeTagIn = node.get_child("TimeBeforeTagIn").get_value<unsigned short>();
		d_timeBeforeTagOut = node.get_child("TimeBeforeTagOut").get_value<unsigned short>();
		d_timeBeforeSafety = node.get_child("TimeBeforeSafety").get_value<unsigned short>();
		d_timeRemoval = node.get_child("TimeRemoval").get_value<unsigned char>();
		d_portBaudRate = node.get_child("PortBaudRate").get_value<unsigned long>();
	}

	std::string SCIELReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "SCIELReaderUnitConfiguration";
	}
}
