/**
 * \file mifarelocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for Mifare card.
 */

#include <boost/property_tree/ptree.hpp>
#include "mifarelocation.hpp"

namespace logicalaccess
{
    MifareLocation::MifareLocation() :
        sector(1),
        block(0),
        byte_(0),
        useMAD(false),
        aid(0x482A)
    {
    }

    MifareLocation::~MifareLocation()
    {
    }

    void MifareLocation::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("Sector", sector);
        node.put("Block", block);
        node.put("Byte", byte_);
        node.put("Sector", sector);
        node.put("Sector", sector);

        boost::property_tree::ptree madnode;
        madnode.put("Use", useMAD);
        madnode.put("AID", aid);
        node.add_child("MAD", madnode);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void MifareLocation::unSerialize(boost::property_tree::ptree& node)
    {
        sector = node.get_child("Sector").get_value<int>();
        block = node.get_child("Block").get_value<int>();
        byte_ = node.get_child("Byte").get_value<int>();

        boost::property_tree::ptree madnode = node.get_child("MAD");
        if (!madnode.empty())
        {
            useMAD = madnode.get_child("Use").get_value<bool>();
            aid = madnode.get_child("AID").get_value<unsigned short>();
        }
    }

    std::string MifareLocation::getDefaultXmlNodeName() const
    {
        return "MifareLocation";
    }

    bool MifareLocation::operator==(const Location& location) const
    {
        if (!Location::operator==(location))
            return false;

        if (typeid(location) != typeid(MifareLocation))
            return false;

        const MifareLocation* mLocation = dynamic_cast<const MifareLocation*>(&location);

        if (!mLocation)
            return false;

        return (sector == mLocation->sector
            && block == mLocation->block
            && byte_ == mLocation->byte_
            && useMAD == mLocation->useMAD
            && aid == mLocation->aid
            );
    }
}