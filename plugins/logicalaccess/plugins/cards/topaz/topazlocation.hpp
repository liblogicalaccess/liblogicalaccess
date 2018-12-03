/**
 * \file topazlocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz Location.
 */

#ifndef LOGICALACCESS_TOPAZLOCATION_HPP
#define LOGICALACCESS_TOPAZLOCATION_HPP

#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/plugins/cards/topaz/lla_cards_topaz_api.hpp>

namespace logicalaccess
{
/**
 * \brief A  Topaz location informations.
 */
class LLA_CARDS_TOPAZ_API TopazLocation : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    TopazLocation();

    /**
     * \brief Destructor.
     */
    virtual ~TopazLocation();

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
        return "Topaz";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
 * \brief The sector.
 */
    int page;

    /**
     * \brief The byte.
     */
    int byte_;
};
}

#endif /* LOGICALACCESS_TOPAZLOCATION_HPP */