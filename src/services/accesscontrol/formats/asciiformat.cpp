/**
 * \file asciiformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ASCII Format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/asciiformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp"

namespace logicalaccess
{
    ASCIIFormat::ASCIIFormat()
        : StaticFormat()
    {
        d_dataType.reset(new BinaryDataType());
        d_dataRepresentation.reset(new BigEndianDataRepresentation());

        d_formatLinear.d_asciiLength = 16;
        d_formatLinear.d_padding = ' ';

        std::shared_ptr<StringDataField> asciiField(new StringDataField());
        asciiField->setName("Value");
        asciiField->setIsIdentifier(true);
        asciiField->setDataLength(d_formatLinear.d_asciiLength * 8);
        asciiField->setPaddingChar(d_formatLinear.d_padding);
        asciiField->setDataRepresentation(d_dataRepresentation);
        asciiField->setDataType(d_dataType);
        d_fieldList.push_back(asciiField);
    }

    ASCIIFormat::~ASCIIFormat()
    {
    }

    unsigned int ASCIIFormat::getDataLength() const
    {
        return (d_formatLinear.d_asciiLength * 8);
    }

    std::string ASCIIFormat::getName() const
    {
        return std::string("ASCII");
    }

	ByteVector ASCIIFormat::getLinearData() const
    {
        ByteVector ret(d_asciiValue.begin(), d_asciiValue.end());

        if (ret.size() < d_formatLinear.d_asciiLength)
        {
            size_t paddingLength = d_formatLinear.d_asciiLength - ret.size();
            for (size_t i = 0; i < paddingLength; i++)
            {
                ret.push_back(d_formatLinear.d_padding);
            }
        }
		return ret;
	}

    void ASCIIFormat::setLinearData(const ByteVector& data)
    {
        if (data.size() >= d_formatLinear.d_asciiLength)
        {
            d_asciiValue = "";
            int asciiValueLength = 0;

            for (int i = d_formatLinear.d_asciiLength - 1; (i >= 0) && (asciiValueLength == 0); i--)
            {
                if (data[i] != d_formatLinear.d_padding)
                {
                    asciiValueLength = i + 1;
                }
            }

            for (int i = 0; i < asciiValueLength; i++)
            {
                d_asciiValue += data[i];
            }

            setASCIIValue(d_asciiValue);
        }
    }

    size_t ASCIIFormat::getFormatLinearData(ByteVector& data) const
    {
        size_t retLength = sizeof(d_formatLinear);
		data.reserve(retLength);
        
		memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));

        return retLength;
    }

    void ASCIIFormat::setFormatLinearData(const ByteVector& data, size_t* indexByte)
    {
		memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
        (*indexByte) += sizeof(d_formatLinear);

        setASCIILength(d_formatLinear.d_asciiLength);
        setPadding(d_formatLinear.d_padding);
    }

    FormatType ASCIIFormat::getType() const
    {
        return FT_ASCII;
    }

    void ASCIIFormat::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getType());
        node.put("ASCIILength", getASCIILength());
        node.put("Padding", getPadding());

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ASCIIFormat::unSerialize(boost::property_tree::ptree& node)
    {
        setASCIILength(node.get_child("ASCIILength").get_value<unsigned int>());
        setPadding(node.get_child("Padding").get_value<unsigned char>());
    }

    std::string ASCIIFormat::getDefaultXmlNodeName() const
    {
        return "ASCIIFormat";
    }

    std::string ASCIIFormat::getASCIIValue() const
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        return field->getValue();
    }

    void ASCIIFormat::setASCIIValue(std::string value)
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        field->setValue(value);
        d_asciiValue = value;
    }

    unsigned int ASCIIFormat::getASCIILength() const
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        return (field->getDataLength() + 7) / 8;
    }

    void ASCIIFormat::setASCIILength(unsigned int length)
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        field->setDataLength(length * 8);
        d_formatLinear.d_asciiLength = length;
    }

    unsigned char ASCIIFormat::getPadding() const
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        return field->getPaddingChar();
    }

    void ASCIIFormat::setPadding(unsigned char padding)
    {
		std::shared_ptr<StringDataField> field = std::dynamic_pointer_cast<StringDataField>(getFieldFromName("Value"));
        field->setPaddingChar(padding);
        d_formatLinear.d_padding = padding;
    }

    bool ASCIIFormat::checkSkeleton(std::shared_ptr<Format> format) const
    {
        bool ret = false;
        if (format)
        {
            std::shared_ptr<ASCIIFormat> pFormat = std::dynamic_pointer_cast<ASCIIFormat>(format);
            if (pFormat)
            {
                ret = (
                    d_formatLinear.d_asciiLength == pFormat->getASCIILength()
                    && d_formatLinear.d_padding == pFormat->getPadding()
                    );
            }
        }
        return ret;
    }

    bool ASCIIFormat::needUserConfigurationToBeUse() const
    {
        return true;
    }
}