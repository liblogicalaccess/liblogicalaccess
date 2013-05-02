/**
 * \file DataField.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Data field.
 */

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/DataField.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include <stdlib.h>


namespace LOGICALACCESS
{
	DataField::DataField()
	{
		d_position = 0;
	}

	DataField::~DataField()
	{
	}

	unsigned int DataField::getDataLength() const
	{
		return d_length;
	}

	void DataField::setPosition(unsigned int position)
	{
		d_position = position;
	}

	unsigned int DataField::getPosition() const
	{
		return d_position;
	}

	void DataField::setName(const std::string& name)
	{
		d_name = name;
	}

	std::string DataField::getName() const
	{
		return d_name;
	}

	void DataField::serialize(boost::property_tree::ptree& node)
	{
		node.put("Name", d_name);
		node.put("Position", d_position);	
	}

	void DataField::unSerialize(boost::property_tree::ptree& node)
	{
		d_name = node.get_child("Name").get_value<std::string>();
		d_position = node.get_child("Position").get_value<unsigned int>();		
	}
}
