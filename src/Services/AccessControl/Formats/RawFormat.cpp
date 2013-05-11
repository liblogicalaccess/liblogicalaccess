/**
 * \file RawFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Raw Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/RawFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "logicalaccess/Services/AccessControl/Encodings/BinaryDataType.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl//Formats/CustomFormat/ASCIIDataField.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/BinaryDataField.h"
#include "logicalaccess/BufferHelper.h"

namespace logicalaccess
{
	RawFormat::RawFormat()
		: StaticFormat()
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());

		boost::shared_ptr<BinaryDataField> field(new BinaryDataField());
		field->setName("RawData");
		field->setIsIdentifier(true);
		field->setDataLength(getFieldLength(field->getName()));		
		field->setPaddingChar(0x00);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	RawFormat::~RawFormat()
	{
	}

	unsigned int RawFormat::getDataLength() const
	{
		return static_cast<unsigned int>(d_rawData.size()) * 8;
	}

	string RawFormat::getName() const
	{
		return string("Raw");
	}

	void RawFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		string ret;
		std::vector<unsigned char> buf = getRawData();

		if (dataLengthBytes >= buf.size())
		{
			memcpy(data, &buf[0], buf.size());
		}
	}

	void RawFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
		std::vector<unsigned char> sbdata = std::vector<unsigned char>(static_cast<const unsigned char*>(data), static_cast<const unsigned char*>(data) + dataLengthBytes);
		setRawData(sbdata);
	}

	size_t RawFormat::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void RawFormat::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		
	}

	FormatType RawFormat::getType() const
	{
		return FT_RAW;
	}

	void RawFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("RawData", BufferHelper::getHex(getRawData()));

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RawFormat::unSerialize(boost::property_tree::ptree& node)
	{
		std::vector<unsigned char> rawbuf = BufferHelper::fromHexString(node.get_child("RawData").get_value<std::string>());
		setRawData(rawbuf);
	}

	std::string RawFormat::getDefaultXmlNodeName() const
	{
		return "RawFormat";
	}

	std::vector<unsigned char> RawFormat::getRawData() const
	{
		boost::shared_ptr<BinaryDataField> field = boost::dynamic_pointer_cast<BinaryDataField>(getFieldFromName("RawData"));
		return field->getValue();
	}

	void RawFormat::setRawData(std::vector<unsigned char>& data)
	{
		boost::shared_ptr<BinaryDataField> field = boost::dynamic_pointer_cast<BinaryDataField>(getFieldFromName("RawData"));
		field->setDataLength(static_cast<unsigned int>(data.size() * 8));
		field->setValue(data);
		d_rawData = data;
	}

	bool RawFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<RawFormat> pFormat = boost::dynamic_pointer_cast<RawFormat>(format);
			if (pFormat)
			{
				ret = true;
			}
		}
		return ret;
	}

	unsigned int RawFormat::getFieldLength(const string& field) const
	{
		unsigned int length = 0;

		if (field == "RawData")
		{
			length = static_cast<unsigned int>(d_rawData.size() * 8);
		}

		return length;
	}

	bool RawFormat::needUserConfigurationToBeUse() const
	{
		return false;
	}
}
