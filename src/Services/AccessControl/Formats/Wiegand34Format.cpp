/**
 * \file Wiegand34Format.cpp
 * \author Arnaud HUMILIER <arnaud.humilier@islog.eu>
 * \brief Wiegand 34 Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace LOGICALACCESS
{
	Wiegand34Format::Wiegand34Format()
		: WiegandFormat()
	{
		d_leftParityType = PT_EVEN;
		d_leftParityLength = 16;
		d_rightParityType = PT_ODD;
		d_rightParityLength = 16;

		boost::shared_ptr<NumberDataField> field(new NumberDataField());
		field->setName("Uid");
		field->setIsIdentifier(true);
		field->setDataLength(32);		
		field->setDataRepresentation(d_dataRepresentation);
		field->setDataType(d_dataType);
		d_fieldList.push_back(field);
	}

	Wiegand34Format::~Wiegand34Format()
	{
	}

	unsigned int Wiegand34Format::getDataLength() const
	{
		return 34;
	}

	string Wiegand34Format::getName() const
	{
		return string("Wiegand 34");
	}

	void Wiegand34Format::getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 1;
		
		convertField(data, dataLengthBytes, &pos, getUid(), 32);
	}

	void Wiegand34Format::setLinearDataWithoutParity(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 1;

		setUid(revertField(data, dataLengthBytes, &pos, 32));
	}

	size_t Wiegand34Format::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void Wiegand34Format::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		// Nothing to do for this format type
	}

	FormatType Wiegand34Format::getType() const
	{
		return FT_WIEGAND34;
	}

	void Wiegand34Format::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Wiegand34Format::unSerialize(boost::property_tree::ptree& node)
	{		
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string Wiegand34Format::getDefaultXmlNodeName() const
	{
		return "Wiegand34Format";
	}

	bool Wiegand34Format::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<Wiegand34Format> pFormat = boost::dynamic_pointer_cast<Wiegand34Format>(format);
			if (pFormat)
			{
				ret = true;
			}
		}
		return ret;
	}
}

