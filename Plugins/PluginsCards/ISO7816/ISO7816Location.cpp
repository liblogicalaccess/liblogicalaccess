/**
 * \file ISO7816Location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for ISO7816 card.
 */

#include "ISO7816Location.h"
#include "logicalaccess/BufferHelper.h"

#include <cstdlib>
#include <cstring>

#include <boost/foreach.hpp>


namespace logicalaccess
{
	ISO7816Location::ISO7816Location()
	{
		memset(dfname, 0x00, sizeof(dfname));
		dfnamelen = 0;
		fileid = 0;
		fileType = IFT_NONE;
		dataObject = 0;
	}

	ISO7816Location::~ISO7816Location()
	{		
	}

	std::vector<unsigned char> ISO7816Location::getLinearData() const
	{
		std::vector<unsigned char> data(16);

		data.insert(data.end(), dfname, dfname + sizeof(dfname));
		data.push_back(static_cast<unsigned char>(dfnamelen));
		BufferHelper::setUShort(data, fileid);
		data.push_back(static_cast<unsigned char>(fileType));
		BufferHelper::setUShort(data, dataObject);

		return data;
	}

	void ISO7816Location::setLinearData(const std::vector<unsigned char>& data, size_t offset)
	{
		EXCEPTION_ASSERT_WITH_LOG(data.size() >= sizeof(dfname), std::invalid_argument, "invalid buffer");

		memcpy(dfname, &data[offset], sizeof(dfname));
		offset += sizeof(dfname);
		dfnamelen = static_cast<int>(data[offset++]);
		fileid = BufferHelper::getUShort(data, offset);
		fileType = static_cast<ISO7816FileType>(data[offset++]);
		dataObject = BufferHelper::getUShort(data, offset);
	}

	size_t ISO7816Location::getDataSize()
	{
		return (sizeof(dfname) + sizeof(fileid) + 1 + sizeof(dataObject));
	}

	void ISO7816Location::serialize(boost::property_tree::ptree& parentNode)
	{
		std::vector<unsigned char> buf(dfname, dfname + sizeof(dfnamelen));
		boost::property_tree::ptree node;

		node.put("DFName", BufferHelper::getHex(buf));
		node.put("FileID", fileid);
		node.put("FileType", fileType);
		node.put("DataObject", dataObject);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ISO7816Location::unSerialize(boost::property_tree::ptree& node)
	{
		std::string dfnamestr = node.get_child("DFName").get_value<std::string>();
		std::vector<unsigned char> buf = BufferHelper::fromHexString(dfnamestr);
		if (buf.size() <= 16)
		{
			dfnamelen = static_cast<int>(buf.size());
			memcpy(dfname, &buf[0], buf.size());
			fileid = node.get_child("FileID").get_value<unsigned short>();
			fileType = static_cast<ISO7816FileType>(node.get_child("FileType").get_value<unsigned int>());
			dataObject = node.get_child("DataObject").get_value<unsigned short>();
		}
	}

	std::string ISO7816Location::getDefaultXmlNodeName() const
	{
		return "ISO7816Location";
	}

	bool ISO7816Location::operator==(const Location& location) const
	{
		if (!Location::operator==(location))
			return false;

		const ISO7816Location* pxLocation = dynamic_cast<const ISO7816Location*>(&location);

		return (fileid == pxLocation->fileid
			&& fileType == pxLocation->fileType
			&& dataObject == pxLocation->dataObject
			&& dfnamelen == pxLocation->dfnamelen
			&& memcmp(dfname, pxLocation->dfname, sizeof(dfname)) == 0
			);
	}
}

