/**
 * \file iso7816location.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO7816 location.
 */

#ifndef LOGICALACCESS_ISO7816LOCATION_HPP
#define LOGICALACCESS_ISO7816LOCATION_HPP

#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/plugins/cards/iso7816/lla_cards_iso7816_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief ISO7816 file type.
 */
typedef enum {
    IFT_NONE            = 0x00,
    IFT_MASTER          = 0x01,
    IFT_DIRECTORY       = 0x02,
    IFT_TRANSPARENT     = 0x04,
    IFT_LINEAR_FIXED    = 0x08,
    IFT_LINEAR_VARIABLE = 0x10,
    IFT_CYCLIC          = 0x20
} ISO7816FileType;

/**
 * \brief A ISO7816 location informations.
 */
class LLA_CARDS_ISO7816_API ISO7816Location : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    ISO7816Location();

    /**
     * \brief Destructor.
     */
    virtual ~ISO7816Location();

    /**
     * \brief Serialize the current object to XML.
     * \param xmlwriter The XML writer.
     * \return The XML stream.
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
        return "ISO7816";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
 * \brief The DF name.
 */
    unsigned char dfname[16];

    /**
     * \brief The DF name length.
     */
    int dfnamelen;

    /**
     * \brief The file identifier.
     */
    unsigned short fileid;

    /**
     * \brief The file type.
     */
    ISO7816FileType fileType;

    /**
     * \brief The data object.
     */
    unsigned short dataObject;
};
}

#endif /* LOGICALACCESS_ISO7816LOCATION_H */