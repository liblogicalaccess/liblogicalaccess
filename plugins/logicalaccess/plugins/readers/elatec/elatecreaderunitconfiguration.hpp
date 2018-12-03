/**
 * \file elatecreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec Reader unit configuration.
 */

#ifndef LOGICALACCESS_ELATECREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_ELATECREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/elatec/lla_readers_elatec_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Elatec reader unit configuration base class.
 */
class LLA_READERS_ELATEC_API ElatecReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    ElatecReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~ElatecReaderUnitConfiguration();

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