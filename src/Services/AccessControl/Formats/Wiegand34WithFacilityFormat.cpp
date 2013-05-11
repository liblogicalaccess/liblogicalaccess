/**
 * \file Wiegand34WithFacilityFormat.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief Wiegand 34 With Facility Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace logicalaccess
{
	Wiegand34WithFacilityFormat::Wiegand34WithFacilityFormat()
		: Wiegand34Format()
	{
		d_formatLinear.d_facilityCode = 0;

		d_fieldList.clear();
		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(16);		
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("FacilityCode");
		field->setDataLength(16);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	Wiegand34WithFacilityFormat::~Wiegand34WithFacilityFormat()
	{
	}

	string Wiegand34WithFacilityFormat::getName() const
	{
		return string("Wiegand 34 With Facility");
	}

	unsigned short int Wiegand34WithFacilityFormat::getFacilityCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void Wiegand34WithFacilityFormat::setFacilityCode(unsigned short int facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}

	void Wiegand34WithFacilityFormat::getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 1;	

		convertField(data, dataLengthBytes, &pos, getFacilityCode(), 16);
		convertField(data, dataLengthBytes, &pos, getUid(), 16);
	}

	void Wiegand34WithFacilityFormat::setLinearDataWithoutParity(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 1;

		setFacilityCode((unsigned short)revertField(data, dataLengthBytes, &pos, 16));
		setUid(revertField(data, dataLengthBytes, &pos, 16));
	}

	size_t Wiegand34WithFacilityFormat::getFormatLinearData(void* data, size_t dataLengthBytes) const
	{
		size_t retLength = sizeof(d_formatLinear);

		if (dataLengthBytes >= retLength)
		{
			memcpy(&reinterpret_cast<unsigned char*>(data)[0], &d_formatLinear, sizeof(d_formatLinear));
		}

		return retLength;
	}

	void Wiegand34WithFacilityFormat::setFormatLinearData(const void* data, size_t* indexByte)
	{
		memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
		(*indexByte) += sizeof(d_formatLinear);

		setFacilityCode(d_formatLinear.d_facilityCode);
	}

	FormatType Wiegand34WithFacilityFormat::getType() const
	{
		return FT_WIEGAND34FACILITY;
	}

	void Wiegand34WithFacilityFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("FacilityCode", getFacilityCode());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Wiegand34WithFacilityFormat::unSerialize(boost::property_tree::ptree& node)
	{		
		setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string Wiegand34WithFacilityFormat::getDefaultXmlNodeName() const
	{
		return "Wiegand34WithFacilityFormat";
	}

	bool Wiegand34WithFacilityFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Wiegand34WithFacilityFormat> pFormat = boost::dynamic_pointer_cast<Wiegand34WithFacilityFormat>(format);
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

