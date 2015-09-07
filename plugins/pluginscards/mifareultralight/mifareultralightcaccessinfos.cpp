/**
 * \file mifareultralightcaccessinfo.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access informations for Mifare Ultralight C card.
 */

#include <boost/property_tree/ptree.hpp>
#include "mifareultralightcaccessinfo.hpp"
#include "mifareultralightcchip.hpp"

namespace logicalaccess
{
    MifareUltralightCAccessInfo::MifareUltralightCAccessInfo()
        : MifareUltralightAccessInfo()
    {
        key.reset(new TripleDESKey());
    }

    MifareUltralightCAccessInfo::~MifareUltralightCAccessInfo()
    {
    }

    std::string MifareUltralightCAccessInfo::getCardType() const
    {
        return CHIP_MIFAREULTRALIGHTC;
    }

    void MifareUltralightCAccessInfo::generateInfos()
    {
        MifareUltralightAccessInfo::generateInfos();

        key->fromString(generateSimpleKey(TRIPLEDES_KEY_SIZE));
    }

    void MifareUltralightCAccessInfo::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        MifareUltralightAccessInfo::serialize(node);

        boost::property_tree::ptree keynode;
        key->serialize(keynode);
        node.add_child("Key", keynode);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void MifareUltralightCAccessInfo::unSerialize(boost::property_tree::ptree& node)
    {
        MifareUltralightAccessInfo::unSerialize(node.get_child(MifareUltralightAccessInfo::getDefaultXmlNodeName()));

        key->unSerialize(node.get_child("Key"), "");
    }

    std::string MifareUltralightCAccessInfo::getDefaultXmlNodeName() const
    {
        return "MifareUltralightCAccessInfo";
    }

    bool MifareUltralightCAccessInfo::operator==(const AccessInfo& ai) const
    {
        if (!MifareUltralightAccessInfo::operator==(ai))
            return false;

        if (typeid(ai) != typeid(MifareUltralightAccessInfo))
            return false;

        const MifareUltralightCAccessInfo* mAi = dynamic_cast<const MifareUltralightCAccessInfo*>(&ai);

        if (!mAi)
            return false;

        return (key == mAi->key);
    }
}