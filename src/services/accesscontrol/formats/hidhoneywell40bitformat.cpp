/**
 * \file HIDHoneywell40BitFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HID Honeywell Format.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>

#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/hidhoneywell40bitformat.hpp>

namespace logicalaccess
{
HIDHoneywell40BitFormat::HIDHoneywell40BitFormat()
    : StaticFormat()
{
    d_dataType.reset(new BinaryDataType());
    d_dataRepresentation.reset(new BigEndianDataRepresentation());

    d_formatLinear.d_facilityCode = 0;

    std::shared_ptr<NumberDataField> field(new NumberDataField());
    field->setName("Uid");
    field->setIsIdentifier(true);
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
    field = std::make_shared<NumberDataField>();
    field->setName("FacilityCode");
    field->setDataLength(12);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
}

HIDHoneywell40BitFormat::~HIDHoneywell40BitFormat()
{
}

unsigned int HIDHoneywell40BitFormat::getDataLength() const
{
    return 40;
}

std::string HIDHoneywell40BitFormat::getName() const
{
    return std::string("HID Honeywell 40-Bit");
}

unsigned short int HIDHoneywell40BitFormat::getFacilityCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    return static_cast<unsigned short int>(field->getValue());
}

void HIDHoneywell40BitFormat::setFacilityCode(unsigned short int facilityCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    field->setValue(facilityCode);
    d_formatLinear.d_facilityCode = facilityCode;
}

size_t HIDHoneywell40BitFormat::getFormatLinearData(ByteVector & /*data*/) const
{
    return 0;
}

void HIDHoneywell40BitFormat::setFormatLinearData(const ByteVector & /*data*/,
                                             size_t * /*indexByte*/)
{
    // DOES NOTHING
}

FormatType HIDHoneywell40BitFormat::getType() const
{
    return FT_HIDHONEYWELL;
}

void HIDHoneywell40BitFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("FacilityCode", getFacilityCode());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void HIDHoneywell40BitFormat::unSerialize(boost::property_tree::ptree &node)
{
    setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
    setUid(node.get_child("Uid").get_value<unsigned short>());
}

std::string HIDHoneywell40BitFormat::getDefaultXmlNodeName() const
{
    return "HIDHoneywell40BitFormat";
}

bool HIDHoneywell40BitFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<HIDHoneywell40BitFormat> pFormat =
            std::dynamic_pointer_cast<HIDHoneywell40BitFormat>(format);
        if (pFormat)
        {
            ret = ((d_formatLinear.d_facilityCode == 0 ||
                    d_formatLinear.d_facilityCode == pFormat->getFacilityCode()));
        }
    }
    return ret;
}

unsigned char HIDHoneywell40BitFormat::getRightParity(const BitsetStream &data,
                                                 unsigned char rpNo)
{
    unsigned char parity = 0x00;

    if (data.getByteSize() != 0)
    {
        std::vector<unsigned int> positions(4);
        positions[0] = 0 + rpNo;
        positions[1] = 8 + rpNo;
        positions[2] = 16 + rpNo;
        positions[3] = 24 + rpNo;
        parity       = Format::calculateParity(data, PT_EVEN, positions);
    }

    return parity;
}

ByteVector HIDHoneywell40BitFormat::getLinearData() const
{
    BitsetStream data;

    data.append(0x0F, 4, 4);

    convertField(data, getFacilityCode(), 12);
    convertField(data, getUid(), 16);

    for (unsigned char i = 0; i < 8; ++i)
    {
        data.append(getRightParity(data, i), 7, 1);
    }
    return data.getData();
}

void HIDHoneywell40BitFormat::setLinearData(const ByteVector &data)
{
    unsigned int pos = 0;
    BitsetStream _data;
    _data.concat(data);

    if (_data.getByteSize() != 0)
    {
        if (_data.getByteSize() * 8 < getDataLength())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
        }

        BitsetStream fixedValue = BitHelper::extract(_data, pos, 4);
        pos += 4;
        if (fixedValue.getData()[0] != 0xF0)
        {
            char exceptionmsg[256];
            sprintf(exceptionmsg,
                    "HID Honeywell 40-Bit: fixed value doesn't match (%x != %x).",
                    fixedValue.getData()[0], 0x0F);
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
        }

        setFacilityCode((unsigned short)revertField(_data, &pos, 12));
        setUid(revertField(_data, &pos, 16));

        for (unsigned char i = 0; i < 8; ++i)
        {
            pos                  = 32 + i;
            unsigned char parity = getRightParity(_data, i);
            if ((unsigned char)((unsigned char)(data[pos / 8] << (pos % 8)) >> 7) !=
                parity)
            {
                char buftmp[64];
                sprintf(buftmp, "Right parity %u format error.", i);
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, buftmp);
            }
        }
    }
}
}
