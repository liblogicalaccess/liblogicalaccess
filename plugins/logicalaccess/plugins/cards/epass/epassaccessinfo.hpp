#pragma once

#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/plugins/cards/epass/lla_cards_epass_api.hpp>

namespace logicalaccess
{
/**
 * An EPassport AccessInfo object that holds MRZ information.
 */
class LLA_CARDS_EPASS_API EPassAccessInfo : public AccessInfo
{
  public:
    EPassAccessInfo()          = default;
    virtual ~EPassAccessInfo() = default;

    void generateInfos() override;

    std::string getCardType() const override;

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &node) override;

    std::string getDefaultXmlNodeName() const override;

    std::string mrz_;
};
}
