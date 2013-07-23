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
		
	}

	void Rwk400ReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Rwk400ReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		
	}

	std::string Rwk400ReaderUnitConfiguration::getDefaultXmlNodeName() const
	{
		return "Rwk400ReaderUnitConfiguration";
	}
}

