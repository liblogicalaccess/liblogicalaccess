/**
 * \file binarydatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Binary Data field.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp"
#include <cstring>
#include <boost/property_tree/ptree.hpp>

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

    BinaryFieldValue::BinaryFieldValue(const std::vector<uint8_t>& buf)
        : Key()
    {
        clear();
        d_buf.clear();

        if (buf.size() != 0)
        {
            d_buf.insert(d_buf.end(), buf.begin(), buf.end());
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
        d_value = BinaryFieldValue(value);
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

	std::vector<uint8_t> BinaryDataField::getLinearData() const
    {
		BitsetStream data;

        //if ((data.size() * 8) < (d_length + _data.getBitSize()))
        //{
        //    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        //}

		size_t fieldDataLengthBytes = (d_length + 7) / 8;
		size_t copyValueLength = (d_value.getLength() > fieldDataLengthBytes) ? fieldDataLengthBytes : d_value.getLength();
        //unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        //memset(paddedBuffer, d_padding, fieldDataLengthBytes);
		BitsetStream paddedBuffer(d_padding, fieldDataLengthBytes);

//#if defined(UNIX)
//		memcpy(paddedBuffer, d_value.getData(), copyValueLength);
//#else
//		memcpy_s(paddedBuffer, fieldDataLengthBytes, d_value.getData(), copyValueLength);
//#endif
		std::vector<uint8_t> tmp;
		tmp.reserve(copyValueLength);
		std::copy(&d_value.getData()[0], &d_value.getData()[0] + copyValueLength, tmp.begin());
		paddedBuffer.concat(tmp);

        convertBinaryData(paddedBuffer, d_length, data);

		return data.getData();
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

    void BinaryDataField::setLinearData(const std::vector<uint8_t>& data)
    {
		BitsetStream _data;
		_data.concat(data);

        if ((_data.getByteSize() * 8) < (d_length + _data.getBitSize()))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        }

        size_t fieldDataLengthBytes = (d_length + 7) / 8;
        //unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        //memset(paddedBuffer, d_padding, fieldDataLengthBytes);
		BitsetStream paddedBuffer(d_padding, fieldDataLengthBytes);

        revertBinaryData(_data, d_length, paddedBuffer);

        d_value = BinaryFieldValue(paddedBuffer.getData());
    }

    bool BinaryDataField::checkSkeleton(std::shared_ptr<DataField> field) const
    {
        bool ret = false;
        if (field)
        {
            std::shared_ptr<BinaryDataField> pField = std::dynamic_pointer_cast<BinaryDataField>(field);
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
        boost::property_tree::ptree node;

        ValueDataField::serialize(node);
        node.put("Value", d_value.toString());
        node.put("Padding", d_padding);

        parentNode.add_child(getDefaultXmlNodeName(), node);
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
