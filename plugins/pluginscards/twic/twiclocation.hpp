/**
 * \file twiclocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic location.
 */

#ifndef LOGICALACCESS_TWICLOCATION_HPP
#define LOGICALACCESS_TWICLOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
/**
 * \brief A Twic location informations.
 */
class LIBLOGICALACCESS_API TwicLocation : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    TwicLocation();

    /**
     * \brief Destructor.
     */
    virtual ~TwicLocation();

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

    /**
     * \brief Get the card type for this location.
     * \return The card type.
     */
    std::string getCardType() override
    {
        return "Twic";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
 * \brief The data object.
 */
    uint64_t dataObject;

    /**
     * \brief The tag. 0x00 means the full data object.
     */
    unsigned char tag;
};
}

#endif /* LOGICALACCESS_TWICLOCATION_HPP */