#pragma once

#include <logicalaccess/cards/accessinfo.hpp>

namespace logicalaccess
{
/**
 * An EPassport AccessInfo object that holds MRZ information.
 */
class LIBLOGICALACCESS_API EPassAccessInfo : public AccessInfo
{
  public:
    virtual void generateInfos() override;

    virtual std::string getCardType() const override;

    virtual void serialize(boost::property_tree::ptree &parentNode) override;

    virtual void unSerialize(boost::property_tree::ptree &node) override;

    virtual std::string getDefaultXmlNodeName() const override;

    std::string mrz_;
};
}
