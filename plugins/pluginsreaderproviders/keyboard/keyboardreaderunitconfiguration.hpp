/**
 * \file keyboardreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard Reader unit configuration.
 */

#ifndef LOGICALACCESS_KEYBOARDREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_KEYBOARDREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
/**
 * \brief The Keyboard reader unit configuration base class.
 */
class LIBLOGICALACCESS_API KeyboardReaderUnitConfiguration
    : public ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    KeyboardReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~KeyboardReaderUnitConfiguration();

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

    std::string getPrefix() const;

    void setPrefix(const std::string &prefix);

    std::string getSuffix() const;

    void setSuffix(const std::string &suffix);

    bool getIsDecimalNumber() const;

    void setIsDecimalNumber(bool isDecimal);

    std::string getKeyboardLayout() const;

    void setKeyboardLayout(const std::string &klayout);

  protected:
    std::string d_prefix;

    std::string d_suffix;

    bool d_isDecimalNumber;

    // Force the keyboard layout to use (ex: US English = 00000409)
    std::string d_klayout;
};
}

#endif