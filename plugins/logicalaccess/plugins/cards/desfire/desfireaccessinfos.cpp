/**
 * \file desfireaccessinfos.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Access informations for DESFire card.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>

namespace logicalaccess
{
DESFireAccessInfo::DESFireAccessInfo()
{
    masterCardKey.reset(new DESFireKey());
    masterCardKey->fromString("");
    masterApplicationKey.reset(new DESFireKey());
    masterApplicationKey->fromString("");
    readKey    = DESFireCrypto::getDefaultKey(DF_KEY_DES);
    writeKey   = DESFireCrypto::getDefaultKey(DF_KEY_DES);
    readKeyno  = 0;
    writeKeyno = 0;
}

DESFireAccessInfo::~DESFireAccessInfo()
{
}

std::string DESFireAccessInfo::getCardType() const
{
    return CHIP_DESFIRE;
}

void DESFireAccessInfo::generateInfos()
{
    LOG(LogLevel::INFOS) << "Generate access information";
    readKey->fromString(generateSimpleKey(readKey->getLength()));
    readKeyno = 2;
    writeKey->fromString(generateSimpleKey(writeKey->getLength()));
    writeKeyno = 1;
    masterApplicationKey->fromString(
        generateSimpleKey(masterApplicationKey->getLength()));
    masterCardKey->fromString(generateSimpleKey(masterCardKey->getLength()));
}

void DESFireAccessInfo::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    boost::property_tree::ptree rknode;
    readKey->serialize(rknode);
    node.add_child("ReadKey", rknode);
    node.put("ReadKeyno", readKeyno);

    boost::property_tree::ptree wknode;
    writeKey->serialize(wknode);
    node.add_child("WriteKey", wknode);
    node.put("WriteKeyno", writeKeyno);

    boost::property_tree::ptree maknode;
    masterApplicationKey->serialize(maknode);
    node.add_child("MasterApplicationKey", maknode);
    boost::property_tree::ptree mcknode;
    masterCardKey->serialize(mcknode);
    node.add_child("MasterCardKey", mcknode);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void DESFireAccessInfo::unSerialize(boost::property_tree::ptree &node)
{
    LOG(LogLevel::INFOS) << "Unserializing access information...";

    readKey->unSerialize(node.get_child("ReadKey"), "");
    readKeyno = node.get_child("ReadKeyno").get_value<unsigned char>();
    writeKey->unSerialize(node.get_child("WriteKey"), "");
    writeKeyno = node.get_child("WriteKeyno").get_value<unsigned char>();
    masterApplicationKey->unSerialize(node.get_child("MasterApplicationKey"), "");
    masterCardKey->unSerialize(node.get_child("MasterCardKey"), "");
}

std::string DESFireAccessInfo::getDefaultXmlNodeName() const
{
    return "DESFireAccessInfo";
}

bool DESFireAccessInfo::operator==(const AccessInfo &ai) const
{
    if (!AccessInfo::operator==(ai))
        return false;

    if (typeid(ai) != typeid(DESFireAccessInfo))
        return false;

    const DESFireAccessInfo *dfAi = dynamic_cast<const DESFireAccessInfo *>(&ai);

    if (!dfAi)
        return false;

    return (readKey == dfAi->readKey && readKeyno == dfAi->readKeyno &&
            writeKey == dfAi->writeKey && writeKeyno == dfAi->writeKeyno &&
            masterApplicationKey == dfAi->masterApplicationKey &&
            masterCardKey == dfAi->masterCardKey);
}
}