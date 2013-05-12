/**
 * \file desfireev1location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for DESFire EV1 card.
 */

#include "desfireev1location.hpp"

namespace logicalaccess
{
	DESFireEV1Location::DESFireEV1Location()
		: DESFireLocation()
	{
		useEV1 = false;
		cryptoMethod = DF_KEY_DES;
		useISO7816 = false;
		applicationFID = 0x0000;
		fileFID = 0x0000;
	}

	DESFireEV1Location::~DESFireEV1Location()
	{
	}

	std::vector<unsigned char> DESFireEV1Location::getLinearData() const
	{
		std::vector<unsigned char> data = DESFireLocation::getLinearData();

		data.push_back((useEV1) ? 1 : 0);
		data.push_back(static_cast<unsigned char>(cryptoMethod));
		data.push_back((useISO7816) ? 1 : 0);
		data.push_back(static_cast<unsigned char>(applicationFID & 0xff));
		data.push_back(static_cast<unsigned char>((applicationFID & 0xff00) >> 8));
		data.push_back(static_cast<unsigned char>(fileFID & 0xff));
		data.push_back(static_cast<unsigned char>((fileFID & 0xff00) >> 8));

		return data;
	}

	void DESFireEV1Location::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		DESFireLocation::setLinearData(data, offset);
		offset += DESFireLocation::getDataSize();

		useEV1 = (data[offset++] == 1);
		cryptoMethod = static_cast<DESFireKeyType>(data[offset++]);
		useISO7816 = (data[offset++] == 1);
		applicationFID = static_cast<unsigned short>(data[offset++] | (data[offset++] << 8));
		fileFID = static_cast<unsigned short>(data[offset++] | (data[offset++] << 8));
	}

	size_t DESFireEV1Location::getDataSize()
	{
		return (DESFireLocation::getDataSize() + 1 + sizeof(cryptoMethod) + 1 + sizeof(applicationFID) + sizeof(fileFID));
	}

	void DESFireEV1Location::serialize(boost::property_tree::ptree& parentNode)
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

	void DESFireEV1Location::unSerialize(boost::property_tree::ptree& node)
	{	
		DESFireLocation::unSerialize(node.get_child(DESFireLocation::getDefaultXmlNodeName()));
		useEV1 = node.get_child("UseEV1").get_value<bool>();
		cryptoMethod = static_cast<DESFireKeyType>(node.get_child("CryptoMethod").get_value<unsigned int>());
		useISO7816 = node.get_child("UseISO7816").get_value<bool>();
		applicationFID = node.get_child("ApplicationFID").get_value<unsigned short>();
		fileFID = node.get_child("FileFID").get_value<unsigned short>();
	}

	std::string DESFireEV1Location::getDefaultXmlNodeName() const
	{
		return "DESFireEV1Location";
	}

	bool DESFireEV1Location::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const DESFireEV1Location* dfLocation = dynamic_cast<const DESFireEV1Location*>(&location);

		return ((!useEV1 || (cryptoMethod == dfLocation->cryptoMethod
				&& (!useISO7816 || (applicationFID == dfLocation->applicationFID && fileFID == dfLocation->fileFID)))
			 )			
			);
	}
}

