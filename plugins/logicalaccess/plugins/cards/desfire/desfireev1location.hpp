/**
 * \file desfirelocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireLocation.
 */

#ifndef LOGICALACCESS_DESFIREEV1LOCATION_HPP
#define LOGICALACCESS_DESFIREEV1LOCATION_HPP

#include <logicalaccess/plugins/cards/desfire/desfirelocation.hpp>

namespace logicalaccess
{
/**
 * \brief A DESFire EV1 location informations.
 */
class LLA_CARDS_DESFIRE_API DESFireEV1Location : public DESFireLocation
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireEV1Location();

    /**
     * \brief Destructor.
     */
    virtual ~DESFireEV1Location();

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
        return "DESFireEV1";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
 * \brief Use EV1 functionalities.
 */
    bool useEV1;

    /**
     * \brief The application crypto method.
     */
    DESFireKeyType cryptoMethod;

    /**
     * \brief Use ISO7816 naming.
     */
    bool useISO7816;

    /**
     * \brief The ISO7816 application FID.
     */
    unsigned short applicationFID;

    /**
     * \brief The ISO7816 file FID.
     */
    unsigned short fileFID;
};
}

#endif /* LOGICALACCESS_DESFIREEV1LOCATION_HPP */