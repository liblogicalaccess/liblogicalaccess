/**
 * \file Wiegand26Format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand 26 Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace logicalaccess
{
	Wiegand26Format::Wiegand26Format()
		: WiegandFormat()
	{
		d_leftParityType = PT_EVEN;
		d_leftParityLength = 12;
		d_rightParityType = PT_ODD;
		d_rightParityLength = 12;

		d_formatLinear.d_facilityCode = 0;
		
		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(16);		
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
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
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned char>(field->getValue());
	}

	void Wiegand26Format::setFacilityCode(unsigned char facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}

	unsigned int Wiegand26Format::getDataLength() const
	{
		return 26;
	}

	string Wiegand26Format::getName() const
	{
		return string("Wiegand 26");
	}

	void Wiegand26Format::getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 1;
		
		convertField(data, dataLengthBytes, &pos, getFacilityCode(), 8);
		convertField(data, dataLengthBytes, &pos, getUid(), 16);
	}

	void Wiegand26Format::setLinearDataWithoutParity(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 1;

		setFacilityCode((unsigned char)revertField(data, dataLengthBytes, &pos, 8));
		setUid(revertField(data, dataLengthBytes, &pos, 16));
	}

	size_t Wiegand26Format::getFormatLinearData(void* data, size_t dataLengthBytes) const
	{
		size_t retLength = sizeof(d_formatLinear);

		if (dataLengthBytes >= retLength)
		{
			unsigned int pos = 0;
			memcpy(&reinterpret_cast<unsigned char*>(data)[pos], &d_formatLinear, sizeof(d_formatLinear));
			pos += sizeof(d_formatLinear);
		}

		return retLength;
	}

	void Wiegand26Format::setFormatLinearData(const void* data, size_t* indexByte)
	{
		memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
		(*indexByte) += sizeof(d_formatLinear);

		setFacilityCode(d_formatLinear.d_facilityCode);
	}

	FormatType Wiegand26Format::getType() const
	{
		return FT_WIEGAND26;
	}

	void Wiegand26Format::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("FacilityCode", getFacilityCode());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Wiegand26Format::unSerialize(boost::property_tree::ptree& node)
	{
		setFacilityCode(node.get_child("FacilityCode").get_value<unsigned char>());
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string Wiegand26Format::getDefaultXmlNodeName() const
	{
		return "Wiegand26Format";
	}

	bool Wiegand26Format::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Wiegand26Format> pFormat = boost::dynamic_pointer_cast<Wiegand26Format>(format);
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
