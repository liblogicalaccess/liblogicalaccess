#pragma once

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGReaderUnitConfiguration : public ReaderUnitConfiguration
{

      public:

        STidPRGReaderUnitConfiguration();

        virtual void resetConfiguration() override;

        virtual void serialize(boost::property_tree::ptree &parentNode) override;

        virtual void unSerialize(boost::property_tree::ptree &node) override;

        virtual std::string getDefaultXmlNodeName() const override;
};
}
