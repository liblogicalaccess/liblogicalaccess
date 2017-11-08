/**
 * \file desfireaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireAccessInfo.
 */

#ifndef LOGICALACCESS_DESFIREACCESSINFO_HPP
#define LOGICALACCESS_DESFIREACCESSINFO_HPP

#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>

namespace logicalaccess
{
/**
 * \brief A DESFire access informations.
 */
class LIBLOGICALACCESS_API DESFireAccessInfo : public AccessInfo
{
  public:
#ifndef SWIG
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;
#endif

    /**
     * \brief Constructor.
     */
    DESFireAccessInfo();

    /**
     * \brief Destructor.
     */
    virtual ~DESFireAccessInfo();

    /**
     * \brief Generate pseudo-random DESFire access informations.
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
     * \brief Equality operator
     * \param ai Access infos to compare.
     * \return True if equals, false otherwise.
     */
    bool operator==(const AccessInfo &ai) const override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
 * \brief The master card key.
 */
    std::shared_ptr<DESFireKey> masterCardKey;

    /**
     * \brief The master application key.
     */
    std::shared_ptr<DESFireKey> masterApplicationKey;

    /**
     * \brief The read key.
     */
    std::shared_ptr<DESFireKey> readKey;

    /**
     * \brief The write key.
     */
    std::shared_ptr<DESFireKey> writeKey;

    /**
     * \brief The read key number.
     */
    unsigned char readKeyno;

    /**
     * \brief The write key number.
     */
    unsigned char writeKeyno;
};
}

#endif /* LOGICALACCESS_DESFIREACCESSINFO_HPP */