/**
 * \file wiegand37withfacilityrightparity2format.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand 37 With Facility and right parity * 2 Format.
 */

#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityrightparity2format.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
	Wiegand37WithFacilityRightParity2Format::Wiegand37WithFacilityRightParity2Format()
		: StaticFormat()
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());

		d_formatLinear.d_facilityCode = 0;

		d_fieldList.clear();
		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(18);		
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

	Wiegand37WithFacilityRightParity2Format::~Wiegand37WithFacilityRightParity2Format()
	{
	}

	unsigned int Wiegand37WithFacilityRightParity2Format::getDataLength() const
	{
		return 37;
	}

	string Wiegand37WithFacilityRightParity2Format::getName() const
	{
		return string("Wiegand 37 With Facility and right parity * 2");
	}

	unsigned short int Wiegand37WithFacilityRightParity2Format::getFacilityCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void Wiegand37WithFacilityRightParity2Format::setFacilityCode(unsigned short int facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}
	
	unsigned char Wiegand37WithFacilityRightParity2Format::getLeftParity(const void* data, size_t dataLengthBytes) const
	{
		return calculateParity(data, dataLengthBytes, PT_ODD, 1, 16);
	}

	unsigned char Wiegand37WithFacilityRightParity2Format::getRightParity1(const void* data, size_t dataLengthBytes) const
	{
		return calculateParity(data, dataLengthBytes, PT_EVEN, 17, 18);
	}

	unsigned char Wiegand37WithFacilityRightParity2Format::getRightParity2(const void* data, size_t dataLengthBytes) const
	{
		return calculateParity(data, dataLengthBytes, PT_ODD, (size_t)0, 36);
	}

	void Wiegand37WithFacilityRightParity2Format::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 1;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif		

		convertField(data, dataLengthBytes, &pos, getFacilityCode(), 16);
		convertField(data, dataLengthBytes, &pos, getUid(), 18);

		if (data != NULL)
		{
			pos = 0;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getLeftParity(data, dataLengthBytes), 7, 1);
			pos = 35; 
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity1(data, dataLengthBytes), 7, 1);
			pos = 36; 
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity2(data, dataLengthBytes), 7, 1);
		}
	}

	void Wiegand37WithFacilityRightParity2Format::setLinearData(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 1;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		setFacilityCode((short)revertField(data, dataLengthBytes, &pos, 16));
		setUid(revertField(data, dataLengthBytes, &pos, 18));

		if (data != NULL)
		{
			pos = 0;
			unsigned char parity = getLeftParity(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Left parity format error.");
			}

			pos = 35;
			parity = getRightParity1(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Right parity 1 format error.");
			}

			pos = 36;
			parity = getRightParity2(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Right parity 1 format error.");
			}
		}
	}

	size_t Wiegand37WithFacilityRightParity2Format::getFormatLinearData(void* data, size_t dataLengthBytes) const
	{
		size_t retLength = sizeof(d_formatLinear);

		if (dataLengthBytes >= retLength)
		{
			memcpy(&reinterpret_cast<unsigned char*>(data)[0], &d_formatLinear, sizeof(d_formatLinear));
		}

		return retLength;
	}

	void Wiegand37WithFacilityRightParity2Format::setFormatLinearData(const void* data, size_t* indexByte)
	{
		memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
		(*indexByte) += sizeof(d_formatLinear);

		setFacilityCode(d_formatLinear.d_facilityCode);
	}

	FormatType Wiegand37WithFacilityRightParity2Format::getType() const
	{
		// Maxime said : I don't want this format listed.
		return FT_UNKNOWN;
	}

	void Wiegand37WithFacilityRightParity2Format::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("FacilityCode", getFacilityCode());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Wiegand37WithFacilityRightParity2Format::unSerialize(boost::property_tree::ptree& node)
	{
		setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string Wiegand37WithFacilityRightParity2Format::getDefaultXmlNodeName() const
	{
		return "Wiegand37WithFacilityRightParity2Format";
	}

	bool Wiegand37WithFacilityRightParity2Format::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Wiegand37WithFacilityRightParity2Format> pFormat = boost::dynamic_pointer_cast<Wiegand37WithFacilityRightParity2Format>(format);
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

