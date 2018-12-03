/**
 * \file rfideasreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RFIDeas Reader unit configuration.
 */

#ifndef LOGICALACCESS_RFIDEASREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RFIDEASREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/rfideas/lla_readers_rfideas_api.hpp>

namespace logicalaccess
{
/**
 * \brief The RFIDeas reader unit configuration base class.
 */
class LLA_READERS_RFIDEAS_API RFIDeasReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    RFIDeasReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~RFIDeasReaderUnitConfiguration();

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