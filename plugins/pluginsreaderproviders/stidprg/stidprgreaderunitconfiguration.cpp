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

    node.put("baudrate", baudrate_);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void STidPRGReaderUnitConfiguration::unSerialize(boost::property_tree::ptree &node)
{
    auto c = node.get_child_optional("baudrate");
    if (c)
    {
        baudrate_ = (*c).get_value<int>();
    }
    else
        baudrate_ = 9600;
}

std::string STidPRGReaderUnitConfiguration::getDefaultXmlNodeName() const
{
    return "STidPRGReaderUnitConfiguration";
}

int STidPRGReaderUnitConfiguration::getBaudrate() const
{
    return baudrate_;
}

void STidPRGReaderUnitConfiguration::setBaudrate(int v)
{
    baudrate_ = v;
}