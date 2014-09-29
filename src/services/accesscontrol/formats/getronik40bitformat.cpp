/**
 * \file getronik40bitformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Getronik 40-Bit Format.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/getronik40bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
	Getronik40BitFormat::Getronik40BitFormat()
		: StaticFormat()
	{
		d_dataType.reset(new BinaryDataType());
		d_dataRepresentation.reset(new BigEndianDataRepresentation());

		d_formatLinear.d_field = 0;

		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(16);		
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
		field = boost::shared_ptr<NumberDataField>(new NumberDataField());
		field->setName("Field");
		field->setDataLength(14);
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	Getronik40BitFormat::~Getronik40BitFormat()
	{
	}

	unsigned int Getronik40BitFormat::getDataLength() const
	{
		return 40;
	}

	string Getronik40BitFormat::getName() const
	{
		return string("Getronik 40-Bit");
	}

	unsigned short int Getronik40BitFormat::getField() const
	{
		boost::shared_ptr<NumberDataField> field = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Field"));
		return static_cast<unsigned short int>(field->getValue());
	}

	void Getronik40BitFormat::setField(unsigned short int field)
	{
		boost::shared_ptr<NumberDataField> nfield = boost::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Field"));
		nfield->setValue(field);
		d_formatLinear.d_field = field;
	}

	size_t Getronik40BitFormat::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void Getronik40BitFormat::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		//DOES NOTHING
	}

	FormatType Getronik40BitFormat::getType() const
	{
		return FT_GETRONIK40BIT;
	}

	void Getronik40BitFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("Field", getField());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Getronik40BitFormat::unSerialize(boost::property_tree::ptree& node)
	{		
		setField(node.get_child("Field").get_value<unsigned short>());
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string Getronik40BitFormat::getDefaultXmlNodeName() const
	{
		return "Getronik40BitFormat";
	}

	bool Getronik40BitFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Getronik40BitFormat> pFormat = boost::dynamic_pointer_cast<Getronik40BitFormat>(format);
			if (pFormat)
			{
				ret = (
					(d_formatLinear.d_field == 0 || d_formatLinear.d_field == pFormat->getField())
					);
			}
		}
		return ret;
	}

	unsigned char Getronik40BitFormat::getRightParity(const void* data, size_t dataLengthBytes) const
	{
		unsigned char parity = 0x00;

		if (data != NULL)
		{			
			parity = calculateParity(data, dataLengthBytes, PT_EVEN, static_cast<size_t>(0), 38);
			size_t pos = 39;
			parity = parity ^ (unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7);
		}

		return parity;
	}

	void Getronik40BitFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 0;

		if(data != NULL)
		{
			BitHelper::writeToBit(data, dataLengthBytes, &pos, 0x2E, 0, 8);
		}
		else
		{
			pos += 4;
		}

		convertField(data, dataLengthBytes, &pos, getUid(), 16);
		convertField(data, dataLengthBytes, &pos, getField(), 14);
		if(data != NULL)
		{
			pos = 39;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, 0x01, 7, 1);
			pos = 38;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity(data, dataLengthBytes), 7, 1);
		}
	}		

	void Getronik40BitFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 0;
		unsigned char fixedValue = 0x00;
		if (data != NULL)
		{
			if (dataLengthBytes*8 < getDataLength())
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
			}

			BitHelper::extract(&fixedValue, 1, data, dataLengthBytes, static_cast<unsigned int>(dataLengthBytes * 8), pos, 8);
			if (fixedValue != 0x2E)
			{
				char exceptionmsg[256];
				sprintf(exceptionmsg, "Getronik 40-Bit: fixed left value doesn't match (%x != %x).", fixedValue, 0x2E);
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
			}
			pos = 39;
			fixedValue = 0x00;
			BitHelper::extract(&fixedValue, 1, data, dataLengthBytes, static_cast<unsigned int>(dataLengthBytes * 8), pos, 1);
			fixedValue = (fixedValue >> 7) & 0x01;
			if (fixedValue != 0x01)
			{
				char exceptionmsg[256];
				sprintf(exceptionmsg, "Getronik 40-Bit: fixed right value doesn't match (%x != %x).", fixedValue, 0x01);
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
			}
			pos = 8;
			d_uid = revertField(data, dataLengthBytes, &pos, 16);
			d_formatLinear.d_field = (unsigned short)revertField(data, dataLengthBytes, &pos, 14);
			pos = 38;
			unsigned char parity = getRightParity(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Right parity format error.");
			}
		}
	}
}

