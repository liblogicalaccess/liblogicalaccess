/**
 * \file mifarelocation.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief MifareLocation.
 */

#ifndef LOGICALACCESS_MIFARELOCATION_HPP
#define LOGICALACCESS_MIFARELOCATION_HPP

#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/plugins/cards/mifare/lla_cards_mifare_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief A  Mifare location informations.
 */
class LLA_CARDS_MIFARE_API MifareLocation : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    MifareLocation();

    /**
     * \brief Destructor.
     */
    virtual ~MifareLocation();

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
        return "Mifare";
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
    int sector;

    /**
     * \brief The block.
     */
    int block;

    /**
     * \brief The byte.
     */
    int byte_;

    /**
     * \brief MAD is used ?
     */
    bool useMAD;

    /**
     * \brief The MAD Application ID.
     */
    unsigned short aid;
};
}

#endif /* LOGICALACCESS_MIFARELOCATION_HPP */