/**
 * \file desfireev1location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for DESFire EV1 card.
 */

#include <boost/property_tree/ptree.hpp>
#include "desfireev1location.hpp"

namespace logicalaccess
{
DESFireEV1Location::DESFireEV1Location()
    : DESFireLocation()
{
    useEV1         = false;
    cryptoMethod   = DF_KEY_DES;
    useISO7816     = false;
    applicationFID = 0x0000;
    fileFID        = 0x0000;
}

DESFireEV1Location::~DESFireEV1Location()
{
}

void DESFireEV1Location::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    DESFireLocation::serialize(node);
    node.put("UseEV1", useEV1);
    node.put("CryptoMethod", cryptoMethod);
    node.put("UseISO7816", useISO7816);
    node.put("ApplicationFID", applicationFID);
    node.put("FileFID", fileFID);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void DESFireEV1Location::unSerialize(boost::property_tree::ptree &node)
{
    DESFireLocation::unSerialize(
        node.get_child(DESFireLocation::getDefaultXmlNodeName()));
    useEV1       = node.get_child("UseEV1").get_value<bool>();
    cryptoMethod = static_cast<DESFireKeyType>(
        node.get_child("CryptoMethod").get_value<unsigned int>());
    useISO7816     = node.get_child("UseISO7816").get_value<bool>();
    applicationFID = node.get_child("ApplicationFID").get_value<unsigned short>();
    fileFID        = node.get_child("FileFID").get_value<unsigned short>();
}

std::string DESFireEV1Location::getDefaultXmlNodeName() const
{
    return "DESFireEV1Location";
}

bool DESFireEV1Location::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    if (typeid(location) != typeid(DESFireEV1Location))
        return false;

    const DESFireEV1Location *dfLocation =
        dynamic_cast<const DESFireEV1Location *>(&location);

    if (!dfLocation)
        return false;

    return ((!useEV1 || (cryptoMethod == dfLocation->cryptoMethod &&
                         (!useISO7816 || (applicationFID == dfLocation->applicationFID &&
                                          fileFID == dfLocation->fileFID)))));
}
}