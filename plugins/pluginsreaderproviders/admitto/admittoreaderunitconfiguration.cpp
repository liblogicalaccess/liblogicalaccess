/**
 * \file admittoreaderunitconfiguration.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief  Admitto reader unit configuration.
 */

#include "admittoreaderunitconfiguration.hpp"
#include "admittoreaderprovider.hpp"

namespace logicalaccess
{
    AdmittoReaderUnitConfiguration::AdmittoReaderUnitConfiguration()
        : ReaderUnitConfiguration(READER_ADMITTO)
    {
        resetConfiguration();
    }

    AdmittoReaderUnitConfiguration::~AdmittoReaderUnitConfiguration()
    {
    }

    void AdmittoReaderUnitConfiguration::resetConfiguration()
    {
    }

    void AdmittoReaderUnitConfiguration::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void AdmittoReaderUnitConfiguration::unSerialize(boost::property_tree::ptree& /*node*/)
    {
        LOG(LogLevel::INFOS) << "Unserializing reader unit configuration...";
    }

    std::string AdmittoReaderUnitConfiguration::getDefaultXmlNodeName() const
    {
        return "AdmittoReaderUnitConfiguration";
    }
}