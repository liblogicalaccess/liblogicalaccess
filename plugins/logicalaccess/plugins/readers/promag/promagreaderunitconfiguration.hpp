/**
 * \file promagreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag Reader unit configuration.
 */

#ifndef LOGICALACCESS_PROMAGREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_PROMAGREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/promag/lla_readers_promag_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Promag reader unit configuration base class.
 */
class LLA_READERS_PROMAG_API PromagReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    PromagReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~PromagReaderUnitConfiguration();

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
