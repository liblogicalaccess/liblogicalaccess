/**
 * \file topazaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Topaz AccessInfo.
 */

#ifndef LOGICALACCESS_TOPAZACCESSINFO_HPP
#define LOGICALACCESS_TOPAZACCESSINFO_HPP

#include "logicalaccess/cards/accessinfo.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief A Topaz access informations.
 */
class LIBLOGICALACCESS_API TopazAccessInfo : public AccessInfo
{
  public:
    /**
     * \brief Constructor.
     */
    TopazAccessInfo();

    /**
     * \brief Destructor.
     */
    virtual ~TopazAccessInfo();

    /**
     * \brief Generate pseudo-random Topaz access informations.
     */
    void generateInfos() override;

    /**
     * \brief Get the card type for this access infos.
     * \return The card type.
     */
    std::string getCardType() const override;

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
     * \brief Equality operator
     * \param ai Access infos to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const AccessInfo &ai) const override;

    /**
 * \brief Page is locked ?
 */
    bool lockPage;
};
}

#endif /* LOGICALACCESS_TOPAZACCESSINFO_HPP */