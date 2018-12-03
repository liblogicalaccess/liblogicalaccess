#pragma once

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/stidprg/lla_readers_stidprg_api.hpp>

namespace logicalaccess
{
class LLA_READERS_STIDPRG_API STidPRGReaderUnitConfiguration : public ReaderUnitConfiguration
{

  public:
    STidPRGReaderUnitConfiguration();

    void resetConfiguration() override;

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &node) override;

    std::string getDefaultXmlNodeName() const override;
};
}
