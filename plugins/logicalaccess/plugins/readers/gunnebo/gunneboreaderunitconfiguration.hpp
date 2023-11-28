/**
 * \file gunneboreaderunitconfiguration.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Gunnebo Reader unit configuration.
 */

#ifndef LOGICALACCESS_GUNNEBOREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_GUNNEBOREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/gunnebo/lla_readers_gunnebo_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Gunnebo reader unit configuration base class.
 */
class LLA_READERS_GUNNEBO_API GunneboReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    GunneboReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~GunneboReaderUnitConfiguration();

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