/**
 * \file libusbreaderunitconfiguration.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief LibUSB Reader unit configuration.
 */

#ifndef LOGICALACCESS_LIBUSBREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_LIBUSBREADERUNITCONFIGURATION_HPP

#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/libusb/lla_readers_libusb_api.hpp>

namespace logicalaccess
{
/**
 * \brief The LibUSB reader unit configuration base class.
 */
class LLA_READERS_LIBUSB_API LibUSBReaderUnitConfiguration : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    LibUSBReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~LibUSBReaderUnitConfiguration();

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
    
    unsigned char getSlot() const;
    
    void setSlot(unsigned char slot);
    
  protected:
  
    unsigned char d_slot;
};
}

#endif