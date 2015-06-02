/**
 * \file valuedatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Value Data field.
 */

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"

#include <cstring>
#include <stdlib.h>

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

    void ValueDataField::convertNumericData(void* data, size_t dataLengthBytes, unsigned int* pos, unsigned long long field, unsigned int fieldlen) const
    {
        unsigned int convertedDataTypeLengthBits = d_dataType->convert(field, fieldlen, NULL, 0);
        if (convertedDataTypeLengthBits > 0)
        {
            size_t convertedDataTypeLengthBytes = (convertedDataTypeLengthBits + 7) / 8;
            unsigned char* convertedDataTypeData = new unsigned char[convertedDataTypeLengthBytes];
            memset(convertedDataTypeData, 0x00, convertedDataTypeLengthBytes);

            if (d_dataType->convert(field, fieldlen, convertedDataTypeData, convertedDataTypeLengthBytes) == convertedDataTypeLengthBits)
            {
                unsigned int convertedDataRepresentationLengthBits = d_dataRepresentation->convertNumeric(convertedDataTypeData, convertedDataTypeLengthBytes, convertedDataTypeLengthBits, NULL, 0);
                if (convertedDataRepresentationLengthBits > 0)
                {
                    size_t convertedDataRepresentationLengthBytes = (convertedDataRepresentationLengthBits + 7) / 8;
                    unsigned char* convertedDataRepresentationData = new unsigned char[convertedDataRepresentationLengthBytes];

                    if (d_dataRepresentation->convertNumeric(convertedDataTypeData, convertedDataTypeLengthBytes, convertedDataTypeLengthBits, convertedDataRepresentationData, convertedDataRepresentationLengthBytes) == convertedDataRepresentationLengthBits)
                    {
                        BitHelper::writeToBit(data, dataLengthBytes, pos, convertedDataRepresentationData, convertedDataRepresentationLengthBytes, convertedDataRepresentationLengthBits, 0, convertedDataRepresentationLengthBits);
                    }

                    delete[] convertedDataRepresentationData;
                }
            }
            delete[] convertedDataTypeData;
        }
    }

    void ValueDataField::convertBinaryData(const void* data, size_t dataLengthBytes, unsigned int* pos, unsigned int fieldlen, void* convertedData, size_t convertedDataLengthBytes) const
    {
        unsigned int convertedDataRepresentationLengthBits = d_dataRepresentation->convertBinary(data, dataLengthBytes, fieldlen, NULL, 0);
        if (convertedDataRepresentationLengthBits > 0)
        {
            size_t convertedDataRepresentationLengthBytes = (convertedDataRepresentationLengthBits + 7) / 8;
            unsigned char* convertedDataRepresentationData = new unsigned char[convertedDataRepresentationLengthBytes];

            if (d_dataRepresentation->convertBinary(data, dataLengthBytes, fieldlen, convertedDataRepresentationData, convertedDataRepresentationLengthBytes) == convertedDataRepresentationLengthBits)
            {
                BitHelper::writeToBit(convertedData, convertedDataLengthBytes, pos, convertedDataRepresentationData, convertedDataRepresentationLengthBytes, convertedDataRepresentationLengthBits, 0, convertedDataRepresentationLengthBits);
            }

            delete[] convertedDataRepresentationData;
        }
    }

    unsigned long long ValueDataField::revertNumericData(const void* data, size_t dataLengthBytes, unsigned int* pos, unsigned int fieldlen) const
    {
        unsigned long long ret = 0;

        unsigned int extractedSizeBits = d_dataType->convert(0, d_dataRepresentation->convertLength(fieldlen), NULL, 0);
        size_t extractedSizeBytes = (extractedSizeBits + 7) / 8;
        unsigned int revertedSizeBits = extractedSizeBits;
        size_t revertedSizeBytes = (revertedSizeBits + 7) / 8;

        unsigned char* extractData = new unsigned char[extractedSizeBytes];
        unsigned char* revertedData = new unsigned char[revertedSizeBytes];
        memset(extractData, 0x00, extractedSizeBytes);
        memset(revertedData, 0x00, revertedSizeBytes);

        unsigned int tmp = 0;
        if ((tmp = BitHelper::extract(extractData, extractedSizeBytes, data, dataLengthBytes, static_cast<unsigned int>(dataLengthBytes * 8), *pos, extractedSizeBits)) > 0)
        {
            if ((tmp = d_dataRepresentation->revertNumeric(extractData, extractedSizeBytes, tmp, revertedData, revertedSizeBytes)) > 0)
            {
                ret = d_dataType->revert(revertedData, revertedSizeBytes, tmp);
            }
        }

        delete[] extractData;
        delete[] revertedData;

        (*pos) += extractedSizeBits;

        return ret;
    }

    void ValueDataField::revertBinaryData(const void* data, size_t dataLengthBytes, unsigned int* pos, unsigned int fieldlen, void* revertedData, size_t revertedDataLengthBytes) const
    {
        unsigned int readSizeBits = fieldlen;
        size_t extractedSizeBytes = (readSizeBits + 7) / 8;
        unsigned int revertedSizeBits = readSizeBits;
        size_t revertedTemporarySizeBytes = (revertedSizeBits + 7) / 8;

        unsigned char* extractData = new unsigned char[extractedSizeBytes];
        unsigned char* revertedTemporaryData = new unsigned char[revertedTemporarySizeBytes];
        memset(extractData, 0x00, extractedSizeBytes);
        memset(revertedTemporaryData, 0x00, revertedTemporarySizeBytes);

        unsigned int tmp = 0;
        if ((tmp = BitHelper::extract(extractData, extractedSizeBytes, data, dataLengthBytes, static_cast<unsigned int>(dataLengthBytes * 8), *pos, readSizeBits)) > 0)
        {
            if ((tmp = d_dataRepresentation->revertBinary(extractData, extractedSizeBytes, tmp, revertedTemporaryData, revertedTemporarySizeBytes)) > 0)
            {

#if defined(UNIX)
              if (revertedDataLengthBytes < revertedTemporarySizeBytes)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The size of the dest buffer is too small for memcpy");
              memcpy(revertedData, revertedTemporaryData, revertedTemporarySizeBytes);
#else
              memcpy_s(revertedData, revertedDataLengthBytes, revertedTemporaryData, revertedTemporarySizeBytes);
#endif
            }
        }

        delete[] extractData;
        delete[] revertedTemporaryData;

        (*pos) += readSizeBits;
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
