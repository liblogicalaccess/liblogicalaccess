/**
 * \file deisterreaderunitconfiguration.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Deister Reader unit configuration.
 */

#ifndef LOGICALACCESS_DEISTERREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_DEISTERREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/deister/lla_readers_deister_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Deister reader unit configuration base class.
 */
class LLA_READERS_DEISTER_API DeisterReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    DeisterReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~DeisterReaderUnitConfiguration();

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