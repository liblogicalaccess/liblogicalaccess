/**
 * \file tlvdatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TLV Data field.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/customformat/tlvdatafield.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
	TLVDataField::TLVDataField()
		: ValueDataField(), d_tag(0)
	{
		d_length = 16;
	}

	TLVDataField::TLVDataField(unsigned char tag)
		: ValueDataField(), d_tag(tag)
	{
		d_length = 16;
	}

	TLVDataField::~TLVDataField()
	{
	}

	void TLVDataField::setTag(unsigned char tag)
	{
		d_tag = tag;
	}

	unsigned char TLVDataField::getTag() const
	{
		return d_tag;
	}

	void TLVDataField::setValue(const std::string& value)
	{
		d_length = 16 + value.size() * 8;
		d_value = value;
	}

	std::string TLVDataField::getValue() const
	{
		return d_value;
	}

	void TLVDataField::setRawValue(const std::vector<unsigned char>& value)
	{
		if (value.size() < 2 || value.size() < static_cast<unsigned int>(2 + value[1]))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
		}
		d_tag = value[0];
		setValue(std::string(value.begin() + 2, value.end() + 2 + value[1]));
	}

	std::vector<unsigned char> TLVDataField::getRawValue() const
	{
		auto value = std::vector<unsigned char>(d_value.begin(), d_value.end());
		value.insert(value.begin(), value.size());
		value.insert(value.begin(), d_tag);
		return value;
	}

	void TLVDataField::getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const
	{
		if ((dataLengthBytes * 8) < (d_length + *pos))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
		}

		size_t fieldDataLengthBytes = (d_length + 7) / 8;
		ByteVector ret = getRawValue();
		convertBinaryData(&ret[0], fieldDataLengthBytes, pos, d_length, data, dataLengthBytes);
	}

	void TLVDataField::setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos)
	{
		if (((dataLengthBytes * 8) - *pos) < 16)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
		}

		unsigned int fieldlen = (dataLengthBytes * 8) - *pos;

		size_t fieldDataLengthBytes = (fieldlen + 7) / 8;
		unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
		memset(paddedBuffer, 0x00, fieldDataLengthBytes);

		revertBinaryData(data, dataLengthBytes, pos, fieldlen, paddedBuffer, fieldDataLengthBytes);

		std::vector<unsigned char> ret(paddedBuffer, paddedBuffer + fieldDataLengthBytes);
		delete[] paddedBuffer;
		setRawValue(ret);
	}

	bool TLVDataField::checkSkeleton(std::shared_ptr<DataField> field) const
	{
		bool ret = false;
		if (field)
		{
			std::shared_ptr<TLVDataField> pField = std::dynamic_pointer_cast<TLVDataField>(field);
			if (pField)
			{
				ret = (pField->getDataLength() == getDataLength() &&
					pField->getTag() == getTag() &&
					(pField->getIsFixedField() || getValue() == "" || pField->getValue() == getValue())
					);
			}
		}

		return ret;
	}

	void TLVDataField::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		ValueDataField::serialize(node);
		node.put("Tag", d_tag);
		node.put("Value", getValue());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void TLVDataField::unSerialize(boost::property_tree::ptree& node)
	{
		ValueDataField::unSerialize(node);
		d_tag = node.get_child("Tag").get_value<unsigned char>();
		setValue(node.get_child("Value").get_value<std::string>());
	}

	std::string TLVDataField::getDefaultXmlNodeName() const
	{
		return "TLVDataField";
	}
}
