/**
 * \file binarydatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Binary Data field.
 */

#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp"

#ifdef __unix__
#include <cstring>
#endif


namespace logicalaccess
{
	BinaryFieldValue::BinaryFieldValue()
		: Key()
	{
		clear();
		d_buf.clear();
	}

	BinaryFieldValue::BinaryFieldValue(const string& str)
		: Key()
	{
		d_buf.clear();
		d_buf.resize((str.size() + 2) / 3);
		clear();		
		fromString(str);
	}

	BinaryFieldValue::BinaryFieldValue(const void* buf, size_t buflen)
		: Key()
	{
		clear();
		d_buf.clear();

		if (buf != NULL)
		{
			d_buf.insert(d_buf.end(), static_cast<const unsigned char*>(buf), static_cast<const unsigned char*>(buf) + buflen);
			d_isEmpty = false;
		}
	}

	void BinaryFieldValue::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void BinaryFieldValue::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);
	}

	std::string BinaryFieldValue::getDefaultXmlNodeName() const
	{
		return "BinaryFieldValue";
	}

	BinaryDataField::BinaryDataField()
		: ValueDataField()
	{
		d_length = 0;
		d_padding = 0x00;
	}

	BinaryDataField::~BinaryDataField()
	{
	}

	void BinaryDataField::setValue(std::vector<unsigned char> value)
	{
		d_value = BinaryFieldValue(&value[0], value.size());
	}

	std::vector<unsigned char> BinaryDataField::getValue() const
	{
		const unsigned char* vdata = d_value.getData();
		return std::vector<unsigned char>(vdata, vdata + d_value.getLength());
	}

	void BinaryDataField::setPaddingChar(unsigned char padding)
	{
		d_padding = padding;
	}

	unsigned char BinaryDataField::getPaddingChar() const
	{
		return d_padding;
	}

	void BinaryDataField::getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const
	{
		if ((dataLengthBytes * 8) < (d_length + *pos))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
		}

		size_t fieldDataLengthBytes = (d_length + 7) / 8;
		unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
		memset(paddedBuffer, d_padding, fieldDataLengthBytes);

#if defined(__unix__)
		if (fieldDataLengthBytes < d_value.getLength())
		  THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The size of the dest buffer is too small for memcpy");
		memcpy(paddedBuffer, d_value.getData(), d_value.getLength());
#else
		memcpy_s(paddedBuffer, fieldDataLengthBytes, d_value.getData(), d_value.getLength());
#endif

		convertBinaryData(paddedBuffer, fieldDataLengthBytes, pos, d_length, data, dataLengthBytes);

		// OLD METHOD WAS NOT HANDLING LSB/MSB CONVERSION
		/*for (size_t i = 0; i < ((d_length + 7) / 8); ++i)
		{
			unsigned char c = d_padding;
			if (i < d_value.getLength())
			{
				c = d_value.getData()[i];
			}

			unsigned int blocklen;
			if ((i+1)*8 > d_length)
			{
				blocklen = d_length % 8;
			}
			else
			{
				blocklen = 8;
			}

			convertNumericData(data, dataLengthBytes, pos, c, blocklen);
		}*/
	}

	void BinaryDataField::setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos)
	{
		if ((dataLengthBytes * 8) < (d_length + *pos))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
		}

		size_t fieldDataLengthBytes = (d_length + 7) / 8;
		unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
		memset(paddedBuffer, d_padding, fieldDataLengthBytes);

		revertBinaryData(data, dataLengthBytes, pos, d_length, paddedBuffer, fieldDataLengthBytes);

		d_value = BinaryFieldValue(paddedBuffer, fieldDataLengthBytes);
	}

	bool BinaryDataField::checkSkeleton(boost::shared_ptr<DataField> field) const
	{
		bool ret = false;
		if (field)
		{
			boost::shared_ptr<BinaryDataField> pField = boost::dynamic_pointer_cast<BinaryDataField>(field);
			if (pField)
			{
				ret = (pField->getDataLength() == getDataLength() &&
					pField->getPaddingChar() == getPaddingChar() &&
					pField->getPosition() == getPosition() &&
					(pField->getIsFixedField() || getValue().size() == 0 || pField->getValue() == getValue())
				);
			}
		}

		return ret;
	}

	void BinaryDataField::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node = parentNode.add(getDefaultXmlNodeName(), "");

		ValueDataField::serialize(node);
		node.put("Value", d_value.toString());
		node.put("Padding", d_padding);
	}

	void BinaryDataField::unSerialize(boost::property_tree::ptree& node)
	{
		ValueDataField::unSerialize(node);
		d_value = BinaryFieldValue(node.get_child("Value").get_value<std::string>());
		d_padding = node.get_child("Padding").get_value<unsigned char>();
	}

	std::string BinaryDataField::getDefaultXmlNodeName() const
	{
		return "BinaryDataField";
	}
}
