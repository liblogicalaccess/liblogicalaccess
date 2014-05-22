/**
 * \file fascn200bitformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FASC-N 200 Bit format.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"


namespace logicalaccess
{
	const unsigned char FASCN200BitFormat::FASCN_SS = 0x0B;
	const unsigned char FASCN200BitFormat::FASCN_FS = 0x0D;
	const unsigned char FASCN200BitFormat::FASCN_ES = 0x0F;

	FASCN200BitFormat::FASCN200BitFormat()
		: StaticFormat()
	{
		memset(&d_formatLinear, 0x00, sizeof(d_formatLinear));
		d_dataType.reset(new BCDNibbleDataType());
		d_dataRepresentation.reset(new NoDataRepresentation());
		
		d_dataType->setBitDataRepresentationType(ET_LITTLEENDIAN);
		d_dataType->setRightParityType(PT_ODD);

		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(24);		
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("AgencyCode");
		field->setDataLength(16);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("SystemCode");
		field->setDataLength(16);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("PersonIdentifier");
		field->setDataLength(40);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("OrganizationalCategory");
		field->setDataLength(4);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("OrganizationalIdentifier");
		field->setDataLength(16);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);

		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("POACategory");
		field->setDataLength(4);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	FASCN200BitFormat::~FASCN200BitFormat()
	{
	}

	unsigned short FASCN200BitFormat::getAgencyCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("AgencyCode"));
		return static_cast<unsigned short>(field->getValue());
	}

	void FASCN200BitFormat::setAgencyCode(unsigned short agencyCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("AgencyCode"));
		field->setValue(agencyCode);
		d_formatLinear.d_agencyCode = agencyCode;
	}

	unsigned short FASCN200BitFormat::getSystemCode() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("SystemCode"));
		return static_cast<unsigned short>(field->getValue());
	}

	void FASCN200BitFormat::setSystemCode(unsigned short systemCode)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("SystemCode"));
		field->setValue(systemCode);
		d_formatLinear.d_systemCode = systemCode;
	}

	unsigned char FASCN200BitFormat::getSerieCode() const
	{
		return d_formatLinear.d_serieCode;
	}

	void FASCN200BitFormat::setSerieCode(unsigned char serieCode)
	{
		d_formatLinear.d_serieCode = serieCode;
	}

	unsigned char FASCN200BitFormat::getCredentialCode() const
	{
		return d_formatLinear.d_credentialCode;
	}

	void FASCN200BitFormat::setCredentialCode(unsigned char credentialCode)
	{
		d_formatLinear.d_credentialCode = credentialCode;
	}

	unsigned long long FASCN200BitFormat::getPersonIdentifier() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("PersonIdentifier"));
		return static_cast<unsigned long long>(field->getValue());
	}

	void FASCN200BitFormat::setPersonIdentifier(unsigned long long personIdentifier)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("PersonIdentifier"));
		field->setValue(personIdentifier);
		d_formatLinear.d_personIdentifier = personIdentifier;
	}

	FASCNOrganizationalCategory FASCN200BitFormat::getOrganizationalCategory() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("OrganizationalCategory"));
		return static_cast<FASCNOrganizationalCategory>(field->getValue());
	}

	void FASCN200BitFormat::setOrganizationalCategory(FASCNOrganizationalCategory orgCategory)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("OrganizationalCategory"));
		field->setValue(orgCategory);
		d_formatLinear.d_orgCategory = orgCategory;
	}

	unsigned short FASCN200BitFormat::getOrganizationalIdentifier() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("OrganizationalIdentifier"));
		return static_cast<unsigned short>(field->getValue());
	}

	void FASCN200BitFormat::setOrganizationalIdentifier(unsigned short orgIdentifier)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("OrganizationalIdentifier"));
		field->setValue(orgIdentifier);
		d_formatLinear.d_orgIdentifier = orgIdentifier;
	}

	FASCNPOAssociationCategory FASCN200BitFormat::getPOACategory() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("POACategory"));
		return static_cast<FASCNPOAssociationCategory>(field->getValue());
	}

	void FASCN200BitFormat::setPOACategory(FASCNPOAssociationCategory poaCategory)
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("POACategory"));
		field->setValue(poaCategory);
		d_formatLinear.d_poaCategory = poaCategory;
	}

	unsigned int FASCN200BitFormat::getDataLength() const
	{
		return 200;
	}

	string FASCN200BitFormat::getName() const
	{
		return string("FASC-N 200 Bit");
	}

	FormatType FASCN200BitFormat::getType() const
	{
		return FT_FASCN200BIT;
	}

	void FASCN200BitFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("AgencyCode", getAgencyCode());
		node.put("SystemCode", getSystemCode());
		node.put("SerieCode", d_formatLinear.d_serieCode);
		node.put("CredentialCode", d_formatLinear.d_credentialCode);
		node.put("PersonIdentifier", getPersonIdentifier());
		node.put("OrganizationalCategory", getOrganizationalCategory());
		node.put("OrganizationalIdentifier", getOrganizationalIdentifier());
		node.put("POACategory", getPOACategory());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void FASCN200BitFormat::unSerialize(boost::property_tree::ptree& node)
	{		
		setAgencyCode(node.get_child("AgencyCode").get_value<short>());
		setSystemCode(node.get_child("SystemCode").get_value<short>());
		setSerieCode(node.get_child("SerieCode").get_value<unsigned char>());
		setCredentialCode(node.get_child("CredentialCode").get_value<unsigned char>());
		setPersonIdentifier(node.get_child("PersonIdentifier").get_value<unsigned long long>());
		setOrganizationalCategory(static_cast<FASCNOrganizationalCategory>(node.get_child("OrganizationalCategory").get_value<unsigned int>()));
		setOrganizationalIdentifier(node.get_child("OrganizationalIdentifier").get_value<short>());
		setPOACategory(static_cast<FASCNPOAssociationCategory>(node.get_child("POACategory").get_value<unsigned int>()));
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string FASCN200BitFormat::getDefaultXmlNodeName() const
	{
		return "FASCN200BitFormat";
	}

	void FASCN200BitFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 0;

		convertField(data, dataLengthBytes, &pos, FASCN_SS, 4);
		convertField(data, dataLengthBytes, &pos, getAgencyCode(), 16);
		convertField(data, dataLengthBytes, &pos, FASCN_FS, 4);
		convertField(data, dataLengthBytes, &pos, getSystemCode(), 16);
		convertField(data, dataLengthBytes, &pos, FASCN_FS, 4);
		convertField(data, dataLengthBytes, &pos, getUid(), 24);
		convertField(data, dataLengthBytes, &pos, FASCN_FS, 4);
		convertField(data, dataLengthBytes, &pos, d_formatLinear.d_serieCode, 4);
		convertField(data, dataLengthBytes, &pos, FASCN_FS, 4);
		convertField(data, dataLengthBytes, &pos, d_formatLinear.d_credentialCode, 4);
		convertField(data, dataLengthBytes, &pos, FASCN_FS, 4);
		convertField(data, dataLengthBytes, &pos, getPersonIdentifier(), 40);
		convertField(data, dataLengthBytes, &pos, getOrganizationalCategory(), 4);
		convertField(data, dataLengthBytes, &pos, getOrganizationalIdentifier(), 16);
		convertField(data, dataLengthBytes, &pos, getPOACategory(), 4);
		convertField(data, dataLengthBytes, &pos, FASCN_ES, 4);
		unsigned char lrc = calculateLRC(data, pos);
		convertField(data, dataLengthBytes, &pos, lrc, 4);
	}

	void FASCN200BitFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 0;
		unsigned char c;

		if (dataLengthBytes*8 < getDataLength())
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
		}

		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_SS)
		{
			char tmpmsg[64];
			sprintf(tmpmsg, "The FASC-N Start Sentinel doesn't match (%x).", c);
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmpmsg);
		}

		setAgencyCode((unsigned short)revertField(data, dataLengthBytes, &pos, 16));
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_FS)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N Field Separator doesn't match after the Agency Code.");
		}

		setSystemCode((unsigned short)revertField(data, dataLengthBytes, &pos, 16));
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_FS)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N Field Separator doesn't match after the System Code.");
		}

		setUid(revertField(data, dataLengthBytes, &pos, 24));
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_FS)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N Field Separator doesn't match after the Credential.");
		}

		setSerieCode((unsigned char)revertField(data, dataLengthBytes, &pos, 4));
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_FS)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N Field Separator doesn't match after the Credential Series.");
		}

		setCredentialCode((unsigned char)revertField(data, dataLengthBytes, &pos, 4));
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_FS)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N Field Separator doesn't match after the Credential Issue.");
		}

		setPersonIdentifier(revertField(data, dataLengthBytes, &pos, 40));

		setOrganizationalCategory((FASCNOrganizationalCategory)revertField(data, dataLengthBytes, &pos, 4));
		setOrganizationalIdentifier((unsigned char)revertField(data, dataLengthBytes, &pos, 16));
		setPOACategory((FASCNPOAssociationCategory)revertField(data, dataLengthBytes, &pos, 4));

		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);
		if (c != FASCN_ES)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The End Sentinel doesn't match.");
		}

		unsigned char lrc = calculateLRC(data, pos);
		c = (unsigned char)revertField(data, dataLengthBytes, &pos, 4);

		if (c != lrc)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N LRC seems wrong.");
		}
	}

	unsigned char FASCN200BitFormat::calculateLRC(const void* data, unsigned int datalenBits) const
	{
		unsigned char lrc = 0x00;

		size_t datalenBytes = (datalenBits + 7) / 8;

		if (datalenBytes > 1)
		{
			unsigned int pos = 0;

			while (pos < datalenBits)
			{
				unsigned char c = (unsigned char)revertField(data, datalenBytes, &pos, 4);
				lrc ^= c;
			}
		}

		return lrc;
	}

	size_t FASCN200BitFormat::getFormatLinearData(void* data, size_t dataLengthBytes) const
	{
		size_t retLength = sizeof(d_formatLinear);

		if (dataLengthBytes >= retLength)
		{
			size_t pos = 0;
			memcpy(&reinterpret_cast<unsigned char*>(data)[pos], &d_formatLinear, sizeof(d_formatLinear));
		}

		return retLength;
	}

	void FASCN200BitFormat::setFormatLinearData(const void* data, size_t* indexByte)
	{
		memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte], sizeof(d_formatLinear));
		(*indexByte) += sizeof(d_formatLinear);

		setAgencyCode(d_formatLinear.d_agencyCode);
		setOrganizationalCategory(d_formatLinear.d_orgCategory);
		setOrganizationalIdentifier(d_formatLinear.d_orgIdentifier);
		setPersonIdentifier(d_formatLinear.d_personIdentifier);
		setPOACategory(d_formatLinear.d_poaCategory);
		setSystemCode(d_formatLinear.d_systemCode);
	}

	bool FASCN200BitFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<FASCN200BitFormat> pFormat = boost::dynamic_pointer_cast<FASCN200BitFormat>(format);
			if (pFormat)
			{
				ret = (
					(d_formatLinear.d_agencyCode == 0 || d_formatLinear.d_agencyCode == pFormat->getAgencyCode())
					&& (d_formatLinear.d_systemCode == 0 || d_formatLinear.d_systemCode == pFormat->getSystemCode())
					);
			}
		}
		return ret;
	}
}
