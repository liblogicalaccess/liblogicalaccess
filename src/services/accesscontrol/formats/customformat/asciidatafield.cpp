/**
 * \file asciidatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ASCII Data field.
 */

#include "logicalaccess/services/accesscontrol/formats/customformat/asciidatafield.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    ASCIIDataField::ASCIIDataField()
        : ValueDataField()
    {
        d_length = 0;
        d_padding = ' ';
    }

    ASCIIDataField::~ASCIIDataField()
    {
    }

    void ASCIIDataField::setValue(const std::string& value)
    {
        d_value = value;
    }

    std::string ASCIIDataField::getValue() const
    {
        return d_value;
    }

    void ASCIIDataField::setPaddingChar(unsigned char padding)
    {
        d_padding = padding;
    }

    unsigned char ASCIIDataField::getPaddingChar() const
    {
        return d_padding;
    }

    void ASCIIDataField::getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const
    {
        if ((dataLengthBytes * 8) < (d_length + *pos))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        }

        size_t fieldDataLengthBytes = (d_length + 7) / 8;
		size_t copyValueLength = (d_value.size() > fieldDataLengthBytes) ? fieldDataLengthBytes : d_value.size();
        unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        memset(paddedBuffer, d_padding, fieldDataLengthBytes);
#ifndef __unix__
		memcpy_s(paddedBuffer, fieldDataLengthBytes, d_value.c_str(), copyValueLength);
#else
		memcpy(paddedBuffer, d_value.c_str(), copyValueLength);
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

    void ASCIIDataField::setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos)
    {
        if ((dataLengthBytes * 8) < (d_length + *pos))
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The data length is too short.");
        }

        size_t fieldDataLengthBytes = (d_length + 7) / 8;
        unsigned char* paddedBuffer = new unsigned char[fieldDataLengthBytes];
        memset(paddedBuffer, d_padding, fieldDataLengthBytes);

        revertBinaryData(data, dataLengthBytes, pos, d_length, paddedBuffer, fieldDataLengthBytes);

        std::vector<unsigned char> ret(paddedBuffer, paddedBuffer + fieldDataLengthBytes);
        delete[] paddedBuffer;
        d_value = BufferHelper::getStdString(ret);
    }

    bool ASCIIDataField::checkSkeleton(std::shared_ptr<DataField> field) const
    {
        bool ret = false;
        if (field)
        {
            std::shared_ptr<ASCIIDataField> pField = std::dynamic_pointer_cast<ASCIIDataField>(field);
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

    void ASCIIDataField::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        ValueDataField::serialize(node);
        node.put("Value", d_value);
        node.put("Padding", d_padding);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ASCIIDataField::unSerialize(boost::property_tree::ptree& node)
    {
        ValueDataField::unSerialize(node);
        d_value = node.get_child("Value").get_value<std::string>();
        d_padding = node.get_child("Padding").get_value<unsigned char>();
    }

    std::string ASCIIDataField::getDefaultXmlNodeName() const
    {
        return "ASCIIDataField";
    }
}