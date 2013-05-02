/**
 * \file MifareLocation.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Location informations for Mifare card.
 */

#include "MifareLocation.h"

namespace LOGICALACCESS
{
	MifareLocation::MifareLocation() :
		sector(1),
		block(0),
		byte(0),
		useMAD(false),
		aid(0x482A)
	{
	}

	MifareLocation::~MifareLocation()
	{		
	}

	std::vector<unsigned char> MifareLocation::getLinearData() const
	{
		std::vector<unsigned char> data;

		data.push_back(static_cast<unsigned char>(sector));
		data.push_back(static_cast<unsigned char>(block));
		data.push_back(static_cast<unsigned char>(byte));
		data.push_back(useMAD ? 0x01 : 0x00);
		BufferHelper::setUShort(data, aid);

		return data;
	}

	void MifareLocation::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		sector = static_cast<int>(data[offset++]);
		block = static_cast<int>(data[offset++]);
		byte = static_cast<int>(data[offset++]);
		useMAD = (data[offset++] == 0x01);
		aid = BufferHelper::getUShort(data, offset);
	}

	size_t MifareLocation::getDataSize()
	{
		return (1 + 1 + 1 + 1 + sizeof(aid));
	}

	void MifareLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("Sector", sector);
		node.put("Block", block);
		node.put("Byte", byte);
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
		byte = node.get_child("Byte").get_value<int>();

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

		const MifareLocation* mLocation = dynamic_cast<const MifareLocation*>(&location);

		return (sector == mLocation->sector
			&& block == mLocation->block
			&& byte == mLocation->byte
			&& useMAD == mLocation->useMAD
			&& aid == mLocation->aid
			);
	}
}

