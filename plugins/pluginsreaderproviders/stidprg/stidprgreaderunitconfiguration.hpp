#pragma once

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API STidPRGReaderUnitConfiguration : public ReaderUnitConfiguration
{

      public:

        STidPRGReaderUnitConfiguration();

	void resetConfiguration() override;

	void serialize(boost::property_tree::ptree &parentNode) override;

	void unSerialize(boost::property_tree::ptree &node) override;

	std::string getDefaultXmlNodeName() const override;
};
}
