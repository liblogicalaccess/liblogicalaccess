/**
 * \file BariumFerritePCSCFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Barium Ferrite PCSC Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/BariumFerritePCSCFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "logicalaccess/Services/AccessControl/Encodings/BinaryDataType.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"


namespace LOGICALACCESS
{
	BariumFerritePCSCFormat::BariumFerritePCSCFormat()
		: StaticFormat()
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());

		d_formatLinear.d_facilityCode = 0;

		boost::shared_ptr<NumberDataField> uidField(new NumberDataField());
		uidField->setName("Uid");
		uidField->setIsIdentifier(true);
		uidField->setDataLength(16);		
		uidField->setDataRepresentation(d_dataRepresentation);
		uidField->setDataType(d_dataType);
		d_fieldList.push_back(uidField);
		boost::shared_ptr<NumberDataField> fcField(new NumberDataField());
		fcField->setName("FacilityCode");
		fcField->setDataLength(12);
		fcField->setDataRepresentation(d_dataRepresentation);
		fcField->setDataType(d_dataType);
		d_fieldList.push_back(fcField);
	}

	BariumFerritePCSCFormat::~BariumFerritePCSCFormat()
	{
	}

	unsigned int BariumFerritePCSCFormat::getDataLength() const
	{
		return 40;
	}

	string BariumFerritePCSCFormat::getName() const
	{
		return string("Barium Ferrite PCSC");
	}

	unsigned short int BariumFerritePCSCFormat::getFacilityCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void BariumFerritePCSCFormat::setFacilityCode(unsigned short int facilityCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
		field->setValue(facilityCode);
		d_formatLinear.d_facilityCode = facilityCode;
	}

	size_t BariumFerritePCSCFormat::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void BariumFerritePCSCFormat::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		//DOES NOTHING
	}

	FormatType BariumFerritePCSCFormat::getType() const
	{
		return FT_BARIUM_FERRITE_PCSC;
	}

	void BariumFerritePCSCFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("FacilityCode", getFacilityCode());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void BariumFerritePCSCFormat::unSerialize(boost::property_tree::ptree& node)
	{
		setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string BariumFerritePCSCFormat::getDefaultXmlNodeName() const
	{
		return "BariumFerritePCSCFormat";
	}

	bool BariumFerritePCSCFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<BariumFerritePCSCFormat> pFormat = boost::dynamic_pointer_cast<BariumFerritePCSCFormat>(format);
			if (pFormat)
			{
				ret = (
					(d_formatLinear.d_facilityCode == 0 || d_formatLinear.d_facilityCode == pFormat->getFacilityCode())
					);
			}
		}
		return ret;
	}

	unsigned char BariumFerritePCSCFormat::calcChecksum(const unsigned char* data, unsigned int datalen) const
	{
		unsigned char checksum = 0x00;

		for (unsigned int i = 0; i < datalen; ++i)
		{
			checksum ^= data[i];
		}

		if ((checksum & 0x0F) == 0x0F)
		{
			checksum += 1;
		}

		return checksum;
	}

	void BariumFerritePCSCFormat::getLinearData(void* data, size_t dataLengthBytes) const
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
			BitHelper::writeToBit(data, dataLengthBytes, &pos, calcChecksum(reinterpret_cast<const unsigned char*>(data), static_cast<unsigned int>((pos + 7) / 8)));
		}
		else
		{
			pos += 8;
		}
	}

	void BariumFerritePCSCFormat::setLinearData(const void* data, size_t dataLengthBytes)
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
				sprintf(exceptionmsg, "Barium Ferrit PCSC: fixed value doesn't match (%x != %x).", fixedValue, 0x0F);
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
			unsigned int checkOffset = static_cast<unsigned int>((pos + 7) / 8);
			unsigned char checksum = calcChecksum(reinterpret_cast<const unsigned char*>(data), checkOffset);
			unsigned char currentChecksum = reinterpret_cast<const unsigned char*>(data)[checkOffset];
			
			if (currentChecksum != checksum)
			{
				char exceptionmsg[256];
				sprintf(exceptionmsg, "Barium Ferrit PCSC: checked doesn't match (%x != %x).", currentChecksum, checksum);
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, exceptionmsg);
			}
		}
		else
		{
			pos += 8;
		}
	}
}

