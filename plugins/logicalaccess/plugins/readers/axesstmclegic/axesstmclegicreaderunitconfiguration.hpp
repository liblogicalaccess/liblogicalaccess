/**
 * \file axesstmclegicreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic Reader unit configuration.
 */

#ifndef LOGICALACCESS_AXESSTMCLEGICREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_AXESSTMCLEGICREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>

namespace logicalaccess
{
/**
 * \brief The AxessTMCLegic reader unit configuration base class.
 */
class LIBLOGICALACCESS_API AxessTMCLegicReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    AxessTMCLegicReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~AxessTMCLegicReaderUnitConfiguration();

    /**
     * \brief Reset the configuration to default values
     */
    void resetConfiguration() override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;
};
}

#endif