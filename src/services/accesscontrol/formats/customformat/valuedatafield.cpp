/**
 * \file valuedatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Value Data field.
 */

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/myexception.hpp"

#include <cstring>
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    ValueDataField::ValueDataField()
        : DataField()
    {
        d_isFixedField = false;
        d_isIdentifier = false;
        d_dataRepresentation.reset(new BigEndianDataRepresentation());
        d_dataType.reset(new BinaryDataType());
    }

    ValueDataField::~ValueDataField()
    {
    }

    void ValueDataField::setDataLength(unsigned int length)
    {
        d_length = length;
    }

    std::shared_ptr<DataRepresentation> ValueDataField::getDataRepresentation() const
    {
        return d_dataRepresentation;
    }

    void ValueDataField::setDataRepresentation(std::shared_ptr<DataRepresentation>& encoding)
    {
        d_dataRepresentation = encoding;
    }

    std::shared_ptr<DataType> ValueDataField::getDataType() const
    {
        return d_dataType;
    }

    void ValueDataField::setDataType(std::shared_ptr<DataType>& encoding)
    {
        d_dataType = encoding;
    }

    void ValueDataField::setIsFixedField(bool isFixed)
    {
        d_isFixedField = isFixed;
    }

    bool ValueDataField::getIsFixedField() const
    {
        return d_isFixedField;
    }

    void ValueDataField::setIsIdentifier(bool isIdentifier)
    {
        d_isIdentifier = isIdentifier;
    }

    bool ValueDataField::getIsIdentifier() const
    {
        return d_isIdentifier;
    }

    void ValueDataField::convertNumericData(BitsetStream& data, unsigned long long field, unsigned int fieldlen) const
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
					BitsetStream convertedDataRepresentationData;

                    if (d_dataRepresentation->convertNumeric(convertedDataTypeData, convertedDataRepresentationData) == convertedDataRepresentationLengthBits)
                    {                       
						data.concat(convertedDataRepresentationData.getData());
                    }
                }
            }
        }
    }

    void ValueDataField::convertBinaryData(const BitsetStream& data, unsigned int fieldlen, BitsetStream& convertedData) const
    {
        unsigned int convertedDataRepresentationLengthBits = d_dataRepresentation->convertBinary(data, BitsetStream());
        if (convertedDataRepresentationLengthBits > 0)
        {
            size_t convertedDataRepresentationLengthBytes = (convertedDataRepresentationLengthBits + 7) / 8;
			BitsetStream convertedDataRepresentationData;

            if (d_dataRepresentation->convertBinary(data, convertedDataRepresentationData) == convertedDataRepresentationLengthBits)
            {
				convertedData.concat(convertedDataRepresentationData.getData());
            }
        }
    }

    unsigned long long ValueDataField::revertNumericData(const BitsetStream& data, unsigned int fieldlen) const
    {
        unsigned long long ret = 0;

        unsigned int extractedSizeBits = d_dataType->convert(0, d_dataRepresentation->convertLength(fieldlen), BitsetStream());
        size_t extractedSizeBytes = (extractedSizeBits + 7) / 8;
        unsigned int revertedSizeBits = extractedSizeBits;
        size_t revertedSizeBytes = (revertedSizeBits + 7) / 8;

		BitsetStream extractData(0x00, extractedSizeBytes);
		BitsetStream revertedData(0x00, revertedSizeBytes);

        if (BitHelper::extract(extractData, data, data.getBitSize() - 1, extractedSizeBits) > 0)
        {
            if (d_dataRepresentation->revertNumeric(extractData, revertedData) > 0)
            {
                ret = d_dataType->revert(revertedData);
            }
        }
        return ret;
    }

    void ValueDataField::revertBinaryData(const BitsetStream& data, unsigned int fieldlen, BitsetStream& revertedData) const
    {
        unsigned int readSizeBits = fieldlen;
        size_t extractedSizeBytes = (readSizeBits + 7) / 8;
        unsigned int revertedSizeBits = readSizeBits;
        size_t revertedTemporarySizeBytes = (revertedSizeBits + 7) / 8;

		BitsetStream extractData(0x00, extractedSizeBytes);
		BitsetStream revertedTemporaryData(0x00, revertedTemporarySizeBytes);
		
        if (BitHelper::extract(extractData, data, data.getBitSize(), readSizeBits) > 0)
        {
            if (d_dataRepresentation->revertBinary(extractData, revertedTemporaryData) > 0)
            {
				if (revertedData.getData().size() < revertedTemporaryData.getData().size())
		            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The size of the dest buffer is too small for memcpy");
				revertedData.writeAt(0, revertedTemporaryData.getData(), 0, revertedTemporaryData.getBitSize());
            }
        }
    }

    void ValueDataField::serialize(boost::property_tree::ptree& node)
    {
        DataField::serialize(node);
        node.put("IsFixedField", d_isFixedField);
        node.put("IsIdentifier", d_isIdentifier);
        node.put("DataRepresentation", d_dataRepresentation->getType());
        node.put("DataType", d_dataType->getType());
        node.put("Length", d_length);
    }

    void ValueDataField::unSerialize(boost::property_tree::ptree& node)
    {
        DataField::unSerialize(node);
        d_isFixedField = node.get_child("IsFixedField").get_value<bool>();
        d_isIdentifier = node.get_child("IsIdentifier").get_value<bool>();
        d_dataRepresentation.reset(DataRepresentation::getByEncodingType(static_cast<EncodingType>(node.get_child("DataRepresentation").get_value<unsigned int>())));
        d_dataType.reset(DataType::getByEncodingType(static_cast<EncodingType>(node.get_child("DataType").get_value<unsigned int>())));
        d_length = node.get_child("Length").get_value<unsigned int>();
    }
}
