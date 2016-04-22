#include <boost/property_tree/ptree.hpp>
#include "stidprgreaderunitconfiguration.hpp"
#include "stidprgreaderprovider.hpp"


using namespace logicalaccess;

STidPRGReaderUnitConfiguration::STidPRGReaderUnitConfiguration()
    : ReaderUnitConfiguration(READER_STIDPRG)
{
}

void STidPRGReaderUnitConfiguration::resetConfiguration()
{

}

void STidPRGReaderUnitConfiguration::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void STidPRGReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
}

std::string STidPRGReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "STidPRGReaderUnitConfiguration";
}
