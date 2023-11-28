/**
 * \file proxlocation.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ProxLocation.
 */

#ifndef LOGICALACCESS_PROXLOCATION_HPP
#define LOGICALACCESS_PROXLOCATION_HPP

#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/plugins/cards/prox/lla_cards_prox_api.hpp>

namespace logicalaccess
{
/**
 * \brief A  Prox location informations.
 */
class LLA_CARDS_PROX_API ProxLocation : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    ProxLocation();

    /**
     * \brief Destructor.
     */
    virtual ~ProxLocation();

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
        return "Prox";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
 * \brief The bit.
 */
    int bit;
};
}

#endif /* LOGICALACCESS_PROXLOCATION_HPP */