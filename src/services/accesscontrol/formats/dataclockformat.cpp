/**
 * \file dataclockformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data clock Format.
 */

#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/littleendiandatarepresentation.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
DataClockFormat::DataClockFormat()
    : StaticFormat()
{
    d_dataType.reset(new BCDNibbleDataType());
    d_dataRepresentation.reset(new LittleEndianDataRepresentation());

    std::shared_ptr<NumberDataField> uidField(new NumberDataField());
    uidField->setName("Uid");
    uidField->setIsIdentifier(true);
    uidField->setDataLength(32);
    uidField->setDataRepresentation(d_dataRepresentation);
    uidField->setDataType(d_dataType);
    d_fieldList.push_back(uidField);
}

DataClockFormat::~DataClockFormat()
{
}

unsigned int DataClockFormat::getDataLength() const
{
    return 36;
}

std::string DataClockFormat::getName() const
{
    return std::string("Data clock");
}

size_t DataClockFormat::getFormatLinearData(ByteVector & /*data*/) const
{
    return 0;
}

void DataClockFormat::setFormatLinearData(const ByteVector & /*data*/,
                                          size_t * /*indexByte*/)
{
    // DOES NOTHING
}

FormatType DataClockFormat::getType() const
{
    return FT_DATACLOCK;
}

void DataClockFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void DataClockFormat::unSerialize(boost::property_tree::ptree &node)
{
    setUid(node.get_child("Uid").get_value<unsigned long long>());
}

std::string DataClockFormat::getDefaultXmlNodeName() const
{
    return "DataClockFormat";
}

bool DataClockFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<DataClockFormat> pFormat =
            std::dynamic_pointer_cast<DataClockFormat>(format);
        if (pFormat)
        {
            ret = true;
        }
    }
    return ret;
}

ByteVector DataClockFormat::getLinearData() const
{
    BitsetStream data;
    convertField(data, getUid(), 32);

    if (data.getByteSize() != 0)
    {
        data.append(getRightParity1(data), 7, 1);
        data.append(getRightParity2(data), 7, 1);
        data.append(getRightParity3(data), 7, 1);
        data.append(getRightParity4(data), 7, 1);
    }
    return data.getData();
}

void DataClockFormat::setLinearData(const ByteVector &data)
{
    unsigned int pos = 0;
    BitsetStream _data;
    _data.concat(data);
    setUid(revertField(_data, &pos, 32));

    if (_data.getByteSize() != 0)
    {
        pos                  = 32;
        unsigned char parity = getRightParity1(_data);
        if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Right parity 1 format error.");
        }

        pos    = 33;
        parity = getRightParity2(_data);
        if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Right parity 2 format error.");
        }

        pos    = 34;
        parity = getRightParity3(_data);
        if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Right parity 3 format error.");
        }

        pos    = 35;
        parity = getRightParity4(_data);
        if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) != parity)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Right parity 4 format error.");
        }
    }
}

unsigned char DataClockFormat::getRightParity(const BitsetStream &data, ParityType type,
                                              std::vector<unsigned int> positions)
{
    unsigned char parity = 0x00;

    if (data.getByteSize() != 0)
    {
        parity = Format::calculateParity(data, type, positions);
    }

    return parity;
}

unsigned char DataClockFormat::getRightParity1(const BitsetStream &data)
{
    std::vector<unsigned int> positions(8);
    positions[0] = 0;
    positions[1] = 4;
    positions[2] = 8;
    positions[3] = 12;
    positions[4] = 16;
    positions[5] = 20;
    positions[6] = 24;
    positions[7] = 28;

    return getRightParity(data, PT_EVEN, positions);
}

unsigned char DataClockFormat::getRightParity2(const BitsetStream &data)
{
    std::vector<unsigned int> positions(8);
    positions[0] = 1;
    positions[1] = 5;
    positions[2] = 9;
    positions[3] = 13;
    positions[4] = 17;
    positions[5] = 21;
    positions[6] = 25;
    positions[7] = 29;

    return getRightParity(data, PT_ODD, positions);
}

unsigned char DataClockFormat::getRightParity3(const BitsetStream &data)
{
    std::vector<unsigned int> positions(8);
    positions[0] = 2;
    positions[1] = 6;
    positions[2] = 10;
    positions[3] = 14;
    positions[4] = 18;
    positions[5] = 22;
    positions[6] = 26;
    positions[7] = 30;

    return getRightParity(data, PT_EVEN, positions);
}

unsigned char DataClockFormat::getRightParity4(const BitsetStream &data)
{
    std::vector<unsigned int> positions(8);
    positions[0] = 3;
    positions[1] = 7;
    positions[2] = 11;
    positions[3] = 15;
    positions[4] = 19;
    positions[5] = 23;
    positions[6] = 27;
    positions[7] = 31;

    return getRightParity(data, PT_EVEN, positions);
}
}