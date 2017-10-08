/**
 * \file stringdatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief String Data field.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
	StringDataField::StringDataField()
		: ValueDataField(), d_padding(0), d_charset("ascii")
    {
        d_length = 0;
    }

	StringDataField::~StringDataField()
    {
    }

	void StringDataField::setValue(const std::string& value)
    {
        d_value = ByteVector(value.begin(), value.end());
    }

	std::string StringDataField::getValue() const
    {
        return std::string(d_value.begin(), d_value.end());
    }

	void StringDataField::setRawValue(const ByteVector& value)
	{
		d_value = value;
	}

	ByteVector StringDataField::getRawValue() const
	{
		return d_value;
	}

	void StringDataField::setCharset(const std::string& charset)
	{
		d_charset = charset;
	}

	std::string StringDataField::getCharset() const
	{
		return d_charset;
	}

	void StringDataField::setPaddingChar(unsigned char padding)
    {
        d_padding = padding;
    }

	unsigned char StringDataField::getPaddingChar() const
    {
        return d_padding;
    }

	void StringDataField::getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const
    {
        if ((dataLengthBytes * 8) < (d_length + *pos))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        }

        size_t fieldDataLengthBytes = (d_length + 7) / 8;
		size_t copyValueLength = (d_value.size() > fieldDataLengthBytes) ? fieldDataLengthBytes : d_value.size();
        unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        memset(paddedBuffer, d_padding, fieldDataLengthBytes);
#if defined(UNIX)
        memcpy(paddedBuffer, &d_value[0], copyValueLength);
#else
        memcpy_s(paddedBuffer, fieldDataLengthBytes, &d_value[0], copyValueLength);
#endif

        convertBinaryData(paddedBuffer, fieldDataLengthBytes, pos, d_length, data, dataLengthBytes);
        delete[] paddedBuffer;

        // OLD METHOD WAS NOT HANDLING LSB/MSB CONVERSION
        /*for (size_t i = 0; i < ((d_length + 7) / 8); ++i)
        {
        unsigned char c = d_padding;
        if (i < d_value.size())
        {
        c = d_value.c_str()[i];
        }

        convertNumericData(data, dataLengthBytes, pos, c, 8);
        }*/
    }

	void StringDataField::setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos)
    {
        if ((dataLengthBytes * 8) < (d_length + *pos))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        }

        size_t fieldDataLengthBytes = (d_length + 7) / 8;
        unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        memset(paddedBuffer, d_padding, fieldDataLengthBytes);

        revertBinaryData(data, dataLengthBytes, pos, d_length, paddedBuffer, fieldDataLengthBytes);

        ByteVector ret(paddedBuffer, paddedBuffer + fieldDataLengthBytes);
        delete[] paddedBuffer;
		d_value = ret;
    }

	bool StringDataField::checkSkeleton(std::shared_ptr<DataField> field) const
    {
        bool ret = false;
        if (field)
        {
			std::shared_ptr<StringDataField> pField = std::dynamic_pointer_cast<StringDataField>(field);
            if (pField)
            {
                ret = (pField->getDataLength() == getDataLength() &&
                    pField->getPaddingChar() == getPaddingChar() &&
                    pField->getPosition() == getPosition() &&
                    (pField->getIsFixedField() || getValue() == "" || pField->getValue() == getValue())
                    );
            }
        }

        return ret;
    }

	void StringDataField::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        ValueDataField::serialize(node);
        node.put("Padding", d_padding);
		node.put("Charset", d_charset);
		std::string strv = getValue();
        if (d_charset != "ascii" && d_charset != "us-ascii" && d_charset != "utf-8")
        {
            strv = BufferHelper::getHex(ByteVector(strv.begin(), strv.end()));
        }
        node.put("Value", strv);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

	void StringDataField::unSerialize(boost::property_tree::ptree& node)
    {
        ValueDataField::unSerialize(node);
        d_padding = node.get_child("Padding").get_value<unsigned char>();
		d_charset = node.get_child("Charset").get_value<std::string>();
		std::string strv = node.get_child("Value").get_value<std::string>();
        if (d_charset != "ascii" && d_charset != "us-ascii" && d_charset != "utf-8")
        {
            d_value = BufferHelper::fromHexString(strv);
        }
        else
        {
            setValue(strv);
        }
    }

	std::string StringDataField::getDefaultXmlNodeName() const
    {
        return "StringDataField";
    }
}
