/**
 * \file DESFireLocation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for DESFire card.
 */

#include "DESFireLocation.h"

namespace logicalaccess
{
	DESFireLocation::DESFireLocation()
	{
		aid = 0x000521;
		file = 0;
		byte = 0;
		securityLevel = CM_ENCRYPT;
	}

	DESFireLocation::~DESFireLocation()
	{
	}

	int DESFireLocation::convertAidToInt(const void* aid)
	{
		// LSB first
		int ret = ((static_cast<const unsigned char*>(aid)[0] & 0xff) |
			((((static_cast<const unsigned char*>(aid)[1] & 0xff) << 8) & 0xff00) |
			(((static_cast<const unsigned char*>(aid)[2] & 0xff)<< 16) & 0xff0000)));

		return ret;
	}

	void DESFireLocation::convertIntToAid(int i, void* aid)
	{
		// LSB first
		static_cast<unsigned char*>(aid)[0] = i & 0xff;
		static_cast<unsigned char*>(aid)[1] = (i >> 8) & 0xff;
		static_cast<unsigned char*>(aid)[2] = (i >> 16) & 0xff;
	}

	std::vector<unsigned char> DESFireLocation::getLinearData() const
	{
		std::vector<unsigned char> data(128);

		BufferHelper::setInt32(data, aid);
		data.push_back(static_cast<unsigned char>(file));
		data.push_back(static_cast<unsigned char>(byte));
		data.push_back(static_cast<unsigned char>(securityLevel));

		return data;
	}

	void DESFireLocation::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		aid = BufferHelper::getInt32(data, offset);
		file = data[offset++];
		byte = data[offset++];
		securityLevel = static_cast<EncryptionMode>(data[offset++]);
	}

	size_t DESFireLocation::getDataSize()
	{
		return (sizeof(aid) + 1 + 1 + 1);
	}

	void DESFireLocation::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node = parentNode.add(getDefaultXmlNodeName(), "");

		node.put("AID", aid);
		node.put("File", file);
		node.put("Byte", byte);
		node.put("SecurityLevel", securityLevel);
	}

	void DESFireLocation::unSerialize(boost::property_tree::ptree& node)
	{	
		aid = node.get_child("AID").get_value<int>();
		file = node.get_child("File").get_value<int>();
		byte = node.get_child("Byte").get_value<int>();
		securityLevel = static_cast<EncryptionMode>(node.get_child("SecurityLevel").get_value<unsigned int>());
	}

	std::string DESFireLocation::getDefaultXmlNodeName() const
	{
		return "DESFireLocation";
	}

	bool DESFireLocation::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const DESFireLocation* dfLocation = dynamic_cast<const DESFireLocation*>(&location);

		return (aid == dfLocation->aid
			&& file == dfLocation->file
			&& byte == dfLocation->byte
			&& securityLevel == dfLocation->securityLevel	
			);
	}
}

