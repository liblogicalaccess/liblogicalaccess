/**
 * \file felicalocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa location.
 */

#ifndef LOGICALACCESS_FELICALOCATION_HPP
#define LOGICALACCESS_FELICALOCATION_HPP

#include "logicalaccess/cards/location.hpp"

namespace logicalaccess
{
/**
 * \brief A FeliCa location informations.
 */
class LIBLOGICALACCESS_API FeliCaLocation : public Location
{
  public:
    /**
     * \brief Constructor.
     */
    FeliCaLocation();

    /**
     * \brief Destructor.
     */
    virtual ~FeliCaLocation();

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
        return "FeliCa";
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const Location &location) const override;

    /**
    * \brief The service code or area code.
    */
    unsigned short code;

    /**
     * \brief The block.
     */
    unsigned short block;
};
}

#endif /* LOGICALACCESS_FELICALOCATION_H */