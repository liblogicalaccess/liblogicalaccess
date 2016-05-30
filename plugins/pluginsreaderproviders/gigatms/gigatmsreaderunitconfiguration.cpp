/**
 * \file gigatmsreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GIGA-TMS reader unit configuration.
 */

#include <boost/property_tree/ptree.hpp>
#include "gigatmsreaderunitconfiguration.hpp"
#include "gigatmsreaderprovider.hpp"

namespace logicalaccess
{
	GigaTMSReaderUnitConfiguration::GigaTMSReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_GIGATMS)
    {
        resetConfiguration();
    }

	GigaTMSReaderUnitConfiguration::~GigaTMSReaderUnitConfiguration()
    {
    }

    void GigaTMSReaderUnitConfiguration::resetConfiguration()
    {
    }

    void GigaTMSReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void GigaTMSReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
        //TODO: Implement this.
    }

    std::string GigaTMSReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "GigaTMSReaderUnitConfiguration";
    }
}