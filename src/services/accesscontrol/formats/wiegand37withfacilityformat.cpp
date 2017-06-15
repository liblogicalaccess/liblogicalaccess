/**
 * \file wiegand37withfacilityformat.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand 37 With Facility Format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
    Wiegand37WithFacilityFormat::Wiegand37WithFacilityFormat()
        : Wiegand37Format()
    {
        d_formatLinear.d_facilityCode = 0;

        d_fieldList.clear();
        std::shared_ptr<NumberDataField> field(new NumberDataField());
        field->setName("Uid");
        field->setIsIdentifier(true);
        field->setDataLength(19);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
        field = std::shared_ptr<NumberDataField>(new NumberDataField());
        field->setName("FacilityCode");
        field->setDataLength(16);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
    }

    Wiegand37WithFacilityFormat::~Wiegand37WithFacilityFormat()
    {
    }

    string Wiegand37WithFacilityFormat::getName() const
    {
        return string("Wiegand 37 With Facility");
    }

    unsigned short int Wiegand37WithFacilityFormat::getFacilityCode() const
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
        return static_cast<unsigned short int>(field->getValue());
    }

    void Wiegand37WithFacilityFormat::setFacilityCode(unsigned short int facilityCode)
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
        field->setValue(facilityCode);
        d_formatLinear.d_facilityCode = facilityCode;
    }

	std::vector<uint8_t> Wiegand37WithFacilityFormat::getLinearDataWithoutParity() const
    {
		BitsetStream data;

        convertField(data, getFacilityCode(), 16);
        convertField(data, getUid(), 19);
		data.insert(0, 0x00, 0, 1);

		return data.getData();
    }

    void Wiegand37WithFacilityFormat::setLinearDataWithoutParity(const std::vector<uint8_t>& data)
    {
        unsigned int pos = 1;
		BitsetStream _data;
		_data.concat(data);

        setFacilityCode((unsigned short)revertField(_data, &pos, 16));
        setUid(revertField(_data, &pos, 19));
    }

    size_t Wiegand37WithFacilityFormat::getFormatLinearData(std::vector<uint8_t>& data) const
    {
		size_t retLength = sizeof(d_formatLinear);

		if (data.capacity() >= retLength)
		{
			memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));
		}

        return retLength;
    }

    void Wiegand37WithFacilityFormat::setFormatLinearData(const std::vector<uint8_t>& data, size_t* indexByte)
    {
        //memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
		memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
		(*indexByte) += sizeof(d_formatLinear);

        setFacilityCode(d_formatLinear.d_facilityCode);
    }

    FormatType Wiegand37WithFacilityFormat::getType() const
    {
        return FT_WIEGAND37FACILITY;
    }

    void Wiegand37WithFacilityFormat::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getType());
        node.put("FacilityCode", getFacilityCode());
        node.put("Uid", getUid());

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void Wiegand37WithFacilityFormat::unSerialize(boost::property_tree::ptree& node)
    {
        setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
        setUid(node.get_child("Uid").get_value<unsigned long long>());
    }

    std::string Wiegand37WithFacilityFormat::getDefaultXmlNodeName() const
    {
        return "Wiegand37WithFacilityFormat";
    }

    bool Wiegand37WithFacilityFormat::checkSkeleton(std::shared_ptr<Format> format) const
    {
        bool ret = false;
        if (format)
        {
            std::shared_ptr<Wiegand37WithFacilityFormat> pFormat = std::dynamic_pointer_cast<Wiegand37WithFacilityFormat>(format);
            if (pFormat)
            {
                ret = (
                    (d_formatLinear.d_facilityCode == 0 || d_formatLinear.d_facilityCode == pFormat->getFacilityCode())
                    );
            }
        }
        return ret;
    }
}