/**
 * \file NumberDataField.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Number Data field.
 */

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"


namespace LOGICALACCESS
{
	NumberDataField::NumberDataField()
		: ValueDataField()
	{
		d_length = 0;
		d_value = 0;
	}

	NumberDataField::~NumberDataField()
	{
	}

	void NumberDataField::setValue(long long value)
	{
		d_value = value;
	}

	long long NumberDataField::getValue() const
	{
		return d_value;
	}	

	void NumberDataField::getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const
	{
		if ((dataLengthBytes * 8) < (d_length + *pos))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The data length is too short.");
		}

		convertNumericData(data, dataLengthBytes, pos, d_value, d_length);
	}

	void NumberDataField::setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos)
	{
		if ((dataLengthBytes * 8) < (d_length + *pos))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The data length is too short.");
		}

		d_value = revertNumericData(data, dataLengthBytes, pos, d_length);
	}

	bool NumberDataField::checkSkeleton(boost::shared_ptr<DataField> field) const
	{
		bool ret = false;
		if (field)
		{
			boost::shared_ptr<NumberDataField> pField = boost::dynamic_pointer_cast<NumberDataField>(field);
			if (pField)
			{
				ret = (pField->getDataLength() == getDataLength() &&
					pField->getPosition() == getPosition() &&
					(pField->getIsFixedField() || getValue() == 0 || pField->getValue() == getValue())
				);
			}
		}

		return ret;
	}

	void NumberDataField::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		ValueDataField::serialize(node);
		node.put("Value", d_value);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void NumberDataField::unSerialize(boost::property_tree::ptree& node)
	{
		ValueDataField::unSerialize(node);
		d_value = node.get_child("Value").get_value<long long>();
	}

	std::string NumberDataField::getDefaultXmlNodeName() const
	{
		return "NumberDataField";
	}
}
