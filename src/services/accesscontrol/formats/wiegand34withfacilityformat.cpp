/**
 * \file wiegand34withfacilityformat.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief Wiegand 34 With Facility Format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>

namespace logicalaccess
{
Wiegand34WithFacilityFormat::Wiegand34WithFacilityFormat()
    : Wiegand34Format()
{
    d_formatLinear.d_facilityCode = 0;

    d_fieldList.clear();
    std::shared_ptr<NumberDataField> field(new NumberDataField());
    field->setName("Uid");
    field->setIsIdentifier(true);
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
    field = std::make_shared<NumberDataField>();
    field->setName("FacilityCode");
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
}

Wiegand34WithFacilityFormat::~Wiegand34WithFacilityFormat()
{
}

std::string Wiegand34WithFacilityFormat::getName() const
{
    return std::string("Wiegand 34 With Facility");
}

unsigned short int Wiegand34WithFacilityFormat::getFacilityCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    return static_cast<unsigned short int>(field->getValue());
}

void Wiegand34WithFacilityFormat::setFacilityCode(unsigned short int facilityCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
    field->setValue(facilityCode);
    d_formatLinear.d_facilityCode = facilityCode;
}

BitsetStream Wiegand34WithFacilityFormat::getLinearDataWithoutParity() const
{
    BitsetStream data;
    data.append(0x00, 0, 1);
    convertField(data, getFacilityCode(), 16);
    convertField(data, getUid(), 16);
    data.append(0x00, 0, 1);
    return data;
}

void Wiegand34WithFacilityFormat::setLinearDataWithoutParity(const ByteVector &data)
{
    unsigned int pos = 1;
    BitsetStream _data;
    _data.concat(data);

    setFacilityCode((unsigned short)revertField(_data, &pos, 16));
    setUid(revertField(_data, &pos, 16));
}

size_t Wiegand34WithFacilityFormat::getFormatLinearData(ByteVector &data) const
{
    size_t retLength = sizeof(d_formatLinear);

    if (data.capacity() >= retLength)
    {
        memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));
    }

    return retLength;
}

void Wiegand34WithFacilityFormat::setFormatLinearData(const ByteVector &data,
                                                      size_t *indexByte)
{
    memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
    (*indexByte) += sizeof(d_formatLinear);

    setFacilityCode(d_formatLinear.d_facilityCode);
}

FormatType Wiegand34WithFacilityFormat::getType() const
{
    return FT_WIEGAND34FACILITY;
}

void Wiegand34WithFacilityFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("FacilityCode", getFacilityCode());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void Wiegand34WithFacilityFormat::unSerialize(boost::property_tree::ptree &node)
{
    setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
    setUid(node.get_child("Uid").get_value<unsigned long long>());
}

std::string Wiegand34WithFacilityFormat::getDefaultXmlNodeName() const
{
    return "Wiegand34WithFacilityFormat";
}

bool Wiegand34WithFacilityFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<Wiegand34WithFacilityFormat> pFormat =
            std::dynamic_pointer_cast<Wiegand34WithFacilityFormat>(format);
        if (pFormat)
        {
            ret = ((d_formatLinear.d_facilityCode == 0 ||
                    d_formatLinear.d_facilityCode == pFormat->getFacilityCode()));
        }
    }
    return ret;
}
}