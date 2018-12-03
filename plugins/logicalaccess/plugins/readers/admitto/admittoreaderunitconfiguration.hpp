/**
 * \file admittoreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto Reader unit configuration.
 */

#ifndef LOGICALACCESS_ADMITTOREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_ADMITTOREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/admitto/lla_readers_admitto_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Admitto reader unit configuration base class.
 */
class LLA_READERS_ADMITTO_API AdmittoReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    AdmittoReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~AdmittoReaderUnitConfiguration();

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