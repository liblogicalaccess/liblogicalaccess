/**
 * \file HIDHoneywellFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HID Honeywell Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/HIDHoneywellFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "logicalaccess/Services/AccessControl/Encodings/BinaryDataType.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace LOGICALACCESS
{
	HIDHoneywellFormat::HIDHoneywellFormat()
		: StaticFormat()
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());

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
		field->setDataLength(12);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	HIDHoneywellFormat::~HIDHoneywellFormat()
	{
	}

	unsigned int HIDHoneywellFormat::getDataLength() const
	{
		return 40;
	}

	string HIDHoneywellFormat::getName() const
	{
		return string("HID Honeywell 40-Bit");
	}

	unsigned short int HIDHoneywellFormat::getFacilityCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void HIDHoneywellFormat::setFacilityCode(unsigned short int facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}

	size_t HIDHoneywellFormat::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void HIDHoneywellFormat::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		//DOES NOTHING
	}

	FormatType HIDHoneywellFormat::getType() const
	{
		return FT_HIDHONEYWELL;
	}

	void HIDHoneywellFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("FacilityCode", getFacilityCode());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void HIDHoneywellFormat::unSerialize(boost::property_tree::ptree& node)
	{
		setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
		setUid(node.get_child("Uid").get_value<unsigned short>());
	}

	std::string HIDHoneywellFormat::getDefaultXmlNodeName() const
	{
		return "HIDHoneywellFormat";
	}

	bool HIDHoneywellFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<HIDHoneywellFormat> pFormat = boost::dynamic_pointer_cast<HIDHoneywellFormat>(format);
			if (pFormat)
			{
				ret = (
					(d_formatLinear.d_facilityCode == 0 || d_formatLinear.d_facilityCode == pFormat->getFacilityCode())
					);
			}
		}
		return ret;
	}

	unsigned char HIDHoneywellFormat::getRightParity(const void* data, size_t dataLengthBytes, unsigned char rpNo) const
	{
		unsigned char parity = 0x00;

		if (data != NULL)
		{
			unsigned int positions[4];
			positions[0] = 0 + rpNo;
			positions[1] = 8 + rpNo;
			positions[2] = 16 + rpNo;
			positions[3] = 24 + rpNo;			
			parity = Format::calculateParity(data, dataLengthBytes, PT_EVEN, positions, sizeof(positions) / sizeof(int));
		}

		return parity;
	}

	void HIDHoneywellFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 0;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		if(data != NULL)
		{
			BitHelper::writeToBit(data, dataLengthBytes, &pos, 0x0F, 4, 4);
		}
		else
		{
			pos += 4;
		}
		convertField(data, dataLengthBytes, &pos, getFacilityCode(), 12);
		convertField(data, dataLengthBytes, &pos, getUid(), 16);
		if (data != NULL)
		{
			for(unsigned char i = 0; i < 8; ++i)
			{
				pos = 32 + i;
				BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity(data, dataLengthBytes, i), 7, 1);
			}
		}
	}		

	void HIDHoneywellFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		unsigned int pos = 0;
		unsigned char fixedValue = 0x00;
		if (data != NULL)
		{
			if (dataLengthBytes*8 < getDataLength())
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Data length too small.");
			}

			BitHelper::extract(&fixedValue, 1, data, dataLengthBytes, static_cast<unsigned int>(dataLengthBytes * 8), pos, 4);
			pos += 4;
			fixedValue = (fixedValue >> 4) & 0x0F;
			if (fixedValue != 0x0F)
			{
				char exceptionmsg[256];
				sprintf(exceptionmsg, "HID Honeywell 40-Bit: fixed value doesn't match (%x != %x).", fixedValue, 0x0F);
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, exceptionmsg);
			}
		}
		else
		{
			pos += 4;
		}
		setFacilityCode((unsigned short)revertField(data, dataLengthBytes, &pos, 12));
		setUid(revertField(data, dataLengthBytes, &pos, 16));
		if (data != NULL)
		{
			for(unsigned char i = 0; i < 8; ++i)
			{
				pos = 32 + i;
				unsigned char parity = getRightParity(data, dataLengthBytes, i);
				if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
				{
					char buftmp[64];
					sprintf(buftmp, "Right parity %u format error.", i);
					THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, buftmp);
				}
			}
		}
	}
}

