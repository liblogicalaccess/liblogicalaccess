/**
 * \file staticformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Static Format Base.
 */

#include <cstring>
#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
    StaticFormat::StaticFormat() :
        d_uid(0)
    {
    }

    std::shared_ptr<DataRepresentation> StaticFormat::getDataRepresentation() const
    {
        return d_dataRepresentation;
    }

    void StaticFormat::setDataRepresentation(std::shared_ptr<DataRepresentation>& encoding)
    {
        d_dataRepresentation = encoding;
    }

    std::shared_ptr<DataType> StaticFormat::getDataType() const
    {
        return d_dataType;
    }

    void StaticFormat::setDataType(std::shared_ptr<DataType>& encoding)
    {
        d_dataType = encoding;
    }

    unsigned long long StaticFormat::getUid() const
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
        return static_cast<unsigned long long>(field->getValue());
    }

    void StaticFormat::setUid(unsigned long long uid)
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
        field->setValue(uid);
        d_uid = uid;
    }

    unsigned char StaticFormat::calculateParity(const BitsetStream& data, ParityType parityType, size_t start, size_t parityLengthBits)
    {
		EXCEPTION_ASSERT_WITH_LOG(data.getByteSize() > start, std::out_of_range, "Bit position is out of bounds.");

        unsigned char parity = 0x00;
        for (size_t i = start; i < (start + parityLengthBits) && i < (data.getByteSize() * 8); i++)
        {
            parity = (parity & 0x01) ^ ((data.getData()[i / 8] >> (7 - (i % 8))) & 0x01);
        }

        switch (parityType)
        {
        case PT_EVEN:
            break;

        case PT_ODD:
            parity = (unsigned char)((~parity) & 0x01);
            break;

        case PT_NONE:
            parity = 0x00;
            break;
        }

        return parity;
    }

    size_t StaticFormat::getEncodingLinearData(std::vector<uint8_t>& data) const
    {
		BitsetStream _data;
		_data.concat(data);
        
		_data.append(static_cast<unsigned char>(d_dataRepresentation->getType()));
		_data.append(static_cast<unsigned char>(d_dataType->getType()));
        
        return 2 * 8;
    }

    void StaticFormat::setEncodingLinearData(const std::vector<uint8_t>& data, size_t* indexByte)
    {
        if (data.size() != 0)
        {
            d_dataRepresentation.reset(DataRepresentation::getByEncodingType(static_cast<EncodingType>(data[(*indexByte)++])));
            d_dataType.reset(DataType::getByEncodingType(static_cast<EncodingType>(data[(*indexByte)++])));
        }
    }

    bool StaticFormat::needUserConfigurationToBeUse() const
    {
        return false;
    }

    void StaticFormat::convertField(BitsetStream& data, unsigned long long field, unsigned int fieldlen) const
    {
        unsigned int convertedDataTypeLengthBits = d_dataType->convert(field, fieldlen, BitsetStream());

        if (convertedDataTypeLengthBits > 0)
        {
            size_t convertedDataTypeLengthBytes = (convertedDataTypeLengthBits + 7) / 8;
			BitsetStream convertedDataTypeData(0x00, convertedDataTypeLengthBytes);

            if (d_dataType->convert(field, fieldlen, convertedDataTypeData) == convertedDataTypeLengthBits)
            {
                unsigned int convertedDataRepresentationLengthBits = d_dataRepresentation->convertNumeric(convertedDataTypeData, BitsetStream());
                if (convertedDataRepresentationLengthBits > 0)
                {
                    size_t convertedDataRepresentationLengthBytes = (convertedDataRepresentationLengthBits + 7) / 8;
					BitsetStream convertedDataRepresentationData(0x00, convertedDataRepresentationLengthBytes);
					
                    if (d_dataRepresentation->convertNumeric(convertedDataTypeData, convertedDataRepresentationData) == convertedDataRepresentationLengthBits)
                    {
						data.concat(convertedDataRepresentationData.getData(), 0, convertedDataRepresentationLengthBits);
                    }
                }
            }
        }
    }

    unsigned long long StaticFormat::revertField(BitsetStream& data, unsigned int* pos, unsigned int fieldlen) const
    {
        unsigned long long ret = 0;

        unsigned int extractedSizeBits = d_dataType->convert(0, d_dataRepresentation->convertLength(fieldlen), BitsetStream());
        size_t extractedSizeBytes = (extractedSizeBits + 7) / 8;
        unsigned int revertedSizeBits = extractedSizeBits;
        size_t revertedSizeBytes = (revertedSizeBits + 7) / 8;

        //unsigned char* extractData = new unsigned char[extractedSizeBytes];
        //unsigned char* revertedData = new unsigned char[revertedSizeBytes];
		BitsetStream extractData;
		BitsetStream revertedData;

        //memset(extractData, 0x00, extractedSizeBytes);
        //memset(revertedData, 0x00, revertedSizeBytes);

        unsigned int tmp = 0;
        if (BitHelper::extract(extractData, data, *pos, extractedSizeBits) > 0)
        {
            if (d_dataRepresentation->revertNumeric(extractData, revertedData) > 0)
            {
                ret = d_dataType->revert(revertedData);
            }
        }
        (*pos) += extractedSizeBits;

        return ret;
    }

    size_t StaticFormat::getSkeletonLinearData(std::vector<uint8_t>& data) const
    {
        size_t index = 0;

        if (data.size() != 0)
        {
            index += getFormatLinearData(data) * 8;
            size_t length = ((index + 7) / 8);

            index += getEncodingLinearData(data);
        }
        else
        {
            index += getFormatLinearData(std::vector<uint8_t>()) * 8;
            index += getEncodingLinearData(std::vector<uint8_t>());
        }

        return index;
    }

    void StaticFormat::setSkeletonLinearData(const std::vector<uint8_t>& data)
    {
        size_t indexByte = 0;
        setFormatLinearData(data, &indexByte);
        setEncodingLinearData(data, &indexByte);
    }
}