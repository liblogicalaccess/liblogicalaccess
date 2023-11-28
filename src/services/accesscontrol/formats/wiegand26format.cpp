/**
 * \file wiegand26format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Wiegand 26 Format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>

namespace logicalaccess
{
Wiegand26Format::Wiegand26Format()
    : WiegandFormat()
{
    d_leftParityType    = PT_EVEN;
    d_leftParityLength  = 12;
    d_rightParityType   = PT_ODD;
    d_rightParityLength = 12;

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
    field->setDataLength(8);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
}

Wiegand26Format::~Wiegand26Format()
{
}

unsigned char Wiegand26Format::getFacilityCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    return static_cast<unsigned char>(field->getValue());
}

void Wiegand26Format::setFacilityCode(unsigned char facilityCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    field->setValue(facilityCode);
    d_formatLinear.d_facilityCode = facilityCode;
}

unsigned int Wiegand26Format::getDataLength() const
{
    return 26;
}

std::string Wiegand26Format::getName() const
{
    return std::string("Wiegand 26");
}

BitsetStream Wiegand26Format::getLinearDataWithoutParity() const
{
    BitsetStream data;
    data.append(0x00, 0, 1);

    convertField(data, getFacilityCode(), 8);
    convertField(data, getUid(), 16);

    data.append(0x00, 0, 1);
    return data;
}

void Wiegand26Format::setLinearDataWithoutParity(const ByteVector &_data)
{
    unsigned int pos = 1;
    BitsetStream data;

    data.concat(_data);
    setFacilityCode((unsigned char)revertField(data, &pos, 8));
    setUid(revertField(data, &pos, 16));
}

size_t Wiegand26Format::getFormatLinearData(ByteVector &data) const
{
    size_t retLength = sizeof(d_formatLinear);

    if (data.size() >= retLength)
    {
        memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));
    }

    return retLength;
}

void Wiegand26Format::setFormatLinearData(const ByteVector &data, size_t *indexByte)
{
    memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
    (*indexByte) += sizeof(d_formatLinear);

    setFacilityCode(d_formatLinear.d_facilityCode);
}

FormatType Wiegand26Format::getType() const
{
    return FT_WIEGAND26;
}

void Wiegand26Format::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("FacilityCode", getFacilityCode());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void Wiegand26Format::unSerialize(boost::property_tree::ptree &node)
{
    setFacilityCode(node.get_child("FacilityCode").get_value<unsigned char>());
    setUid(node.get_child("Uid").get_value<unsigned long long>());
}

std::string Wiegand26Format::getDefaultXmlNodeName() const
{
    return "Wiegand26Format";
}

bool Wiegand26Format::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<Wiegand26Format> pFormat =
            std::dynamic_pointer_cast<Wiegand26Format>(format);
        if (pFormat)
        {
            ret = ((d_formatLinear.d_facilityCode == 0 ||
                    d_formatLinear.d_facilityCode == pFormat->getFacilityCode()));
        }
    }
    return ret;
}
}