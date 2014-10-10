/**
 * \file staticformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Static Format Base.
 */

#include "logicalaccess/services/accesscontrol/formats/staticformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
    StaticFormat::StaticFormat() :
        d_uid(0)
    {
    }

    boost::shared_ptr<DataRepresentation> StaticFormat::getDataRepresentation() const
    {
        return d_dataRepresentation;
    }

    void StaticFormat::setDataRepresentation(boost::shared_ptr<DataRepresentation>& encoding)
    {
        d_dataRepresentation = encoding;
    }

    boost::shared_ptr<DataType> StaticFormat::getDataType() const
    {
        return d_dataType;
    }

    void StaticFormat::setDataType(boost::shared_ptr<DataType>& encoding)
    {
        d_dataType = encoding;
    }

    unsigned long long StaticFormat::getUid() const
    {
        boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
        return static_cast<unsigned long long>(field->getValue());
    }

    void StaticFormat::setUid(unsigned long long uid)
    {
        boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Uid"));
        field->setValue(uid);
        d_uid = uid;
    }

    unsigned char StaticFormat::calculateParity(const void* data, size_t dataLengthBytes, ParityType parityType, size_t start, size_t parityLengthBits)
    {
        unsigned char parity = 0x00;
        for (size_t i = start; i < (start + parityLengthBits) && i < (dataLengthBytes * 8); i++)
        {
            parity = (unsigned char)((parity & 0x01) ^ ((unsigned char)(reinterpret_cast<const char*>(data)[i / 8] >> (7 - (i % 8))) & 0x01));
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

    size_t StaticFormat::getEncodingLinearData(void *data, size_t dataLengthBytes) const
    {
        if (data != NULL)
        {
            EXCEPTION_ASSERT_WITH_LOG(dataLengthBytes >= 2, std::invalid_argument, "dataLengthBytes must be at least 2");

            if (data != NULL)
            {
                reinterpret_cast<char*>(data)[0] = static_cast<char>(d_dataRepresentation->getType());
                reinterpret_cast<char*>(data)[1] = static_cast<char>(d_dataType->getType());
            }
        }

        return 2 * 8;
    }

    void StaticFormat::setEncodingLinearData(const void* data, size_t* indexByte)
    {
        if (data != NULL)
        {
            d_dataRepresentation.reset(DataRepresentation::getByEncodingType(static_cast<EncodingType>(reinterpret_cast<const char*>(data)[(*indexByte)++])));
            d_dataType.reset(DataType::getByEncodingType(static_cast<EncodingType>(reinterpret_cast<const char*>(data)[(*indexByte)++])));
        }
    }

    bool StaticFormat::needUserConfigurationToBeUse() const
    {
        return false;
    }

    void StaticFormat::convertField(void* data, size_t dataLengthBytes, unsigned int* pos, unsigned long long field, unsigned int fieldlen) const
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
                    memset(convertedDataRepresentationData, 0x00, convertedDataRepresentationLengthBytes);

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

    unsigned long long StaticFormat::revertField(const void* data, size_t dataLengthBytes, unsigned int* pos, unsigned int fieldlen) const
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
        if ((tmp = BitHelper::extract(extractData, extractedSizeBytes, data, dataLengthBytes, getDataLength(), *pos, extractedSizeBits)) > 0)
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

    size_t StaticFormat::getSkeletonLinearData(void* data, size_t dataLengthBytes) const
    {
        size_t index = 0;

        if (data != NULL)
        {
            index += getFormatLinearData(data, dataLengthBytes) * 8;
            size_t length = ((index + 7) / 8);

            index += getEncodingLinearData(reinterpret_cast<char*>(data)+length, dataLengthBytes - length);
        }
        else
        {
            index += getFormatLinearData(NULL, 0) * 8;
            index += getEncodingLinearData(NULL, 0);
        }

        return index;
    }

    void StaticFormat::setSkeletonLinearData(const void* data, size_t /*dataLengthBytes*/)
    {
        size_t indexByte = 0;

        setFormatLinearData(data, &indexByte);
        setEncodingLinearData(data, &indexByte);
    }
}