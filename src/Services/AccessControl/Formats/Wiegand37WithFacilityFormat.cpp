/**
 * \file Wiegand37WithFacilityFormat.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>, Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Wiegand 37 With Facility Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace LOGICALACCESS
{
	Wiegand37WithFacilityFormat::Wiegand37WithFacilityFormat()
		: Wiegand37Format()
	{
		d_formatLinear.d_facilityCode = 0;

		d_fieldList.clear();
		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(19);		
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

	Wiegand37WithFacilityFormat::~Wiegand37WithFacilityFormat()
	{
	}

	string Wiegand37WithFacilityFormat::getName() const
	{
		return string("Wiegand 37 With Facility");
	}

	unsigned short int Wiegand37WithFacilityFormat::getFacilityCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void Wiegand37WithFacilityFormat::setFacilityCode(unsigned short int facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}

	void Wiegand37WithFacilityFormat::getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 1;

		convertField(data, dataLengthBytes, &pos, getFacilityCode(), 16);
		convertField(data, dataLengthBytes, &pos, getUid(), 19);		
	}

	void Wiegand37WithFacilityFormat::setLinearDataWithoutParity(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 1;

		setFacilityCode((unsigned short)revertField(data, dataLengthBytes, &pos, 16));
		setUid(revertField(data, dataLengthBytes, &pos, 19));
	}

	size_t Wiegand37WithFacilityFormat::getFormatLinearData(void* data, size_t dataLengthBytes) const
	{
		size_t retLength = sizeof(d_formatLinear);

		if (dataLengthBytes >= retLength)
		{
			memcpy(&reinterpret_cast<unsigned char*>(data)[0], &d_formatLinear, sizeof(d_formatLinear));
		}

		return retLength;
	}

	void Wiegand37WithFacilityFormat::setFormatLinearData(const void* data, size_t* indexByte)
	{
		memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
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

	bool Wiegand37WithFacilityFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Wiegand37WithFacilityFormat> pFormat = boost::dynamic_pointer_cast<Wiegand37WithFacilityFormat>(format);
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

