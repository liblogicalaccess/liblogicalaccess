/**
 * \file CustomFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Custom Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/CustomFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/ASCIIDataField.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/ParityDataField.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/BinaryDataField.h"
#include "logicalaccess/BufferHelper.h"

#include <cstring>
#include <stdlib.h>

#include <boost/foreach.hpp>

namespace LOGICALACCESS
{
	CustomFormat::CustomFormat()
		: Format()
	{
		d_name = "Custom";
	}

	CustomFormat::~CustomFormat()
	{
	}	

	unsigned int CustomFormat::getDataLength() const
	{
		unsigned int dataLength = 0;
		for(std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); i != d_fieldList.end(); ++i)
		{
			unsigned int maxLength = (*i)->getPosition() + (*i)->getDataLength();
			if (maxLength > dataLength)
			{
				dataLength = maxLength;
			}
		}

		return dataLength;
	}

	void CustomFormat::setName(const string& name)
	{
		d_name = name;
	}

	string CustomFormat::getName() const
	{
		return d_name;
	}	

	void CustomFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 0;
		std::list<boost::shared_ptr<DataField> > sortedFieldList = d_fieldList;
		sortedFieldList.sort(FieldSortPredicate);
		memset(data, 0x00, dataLengthBytes);
		for(std::list<boost::shared_ptr<DataField> >::const_iterator i = sortedFieldList.begin(); i != sortedFieldList.end(); ++i)
		{
			pos = (*i)->getPosition();
			(*i)->getLinearData(data, dataLengthBytes, &pos);
		}
	}

	void CustomFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
		unsigned int pos = 0;
		std::list<boost::shared_ptr<DataField> > sortedFieldList = d_fieldList;
		sortedFieldList.sort(FieldSortPredicate);
		for(std::list<boost::shared_ptr<DataField> >::iterator i = sortedFieldList.begin(); i != sortedFieldList.end(); ++i)
		{
			pos = (*i)->getPosition();
			(*i)->setLinearData(data, dataLengthBytes, &pos);
		}
	}

	void CustomFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("Name", d_name);
		
		boost::property_tree::ptree fnode;
		for (std::list<boost::shared_ptr<DataField> >::iterator i = d_fieldList.begin(); i != d_fieldList.end(); ++i)
		{
			(*i)->serialize(fnode);
		}
		node.add_child("Fields", fnode);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void CustomFormat::unSerialize(boost::property_tree::ptree& node)
	{
		d_fieldList.clear();
		d_name = node.get_child("Name").get_value<std::string>();
		BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node.get_child("Fields"))
		{
			// TODO: improve the unserialization process to be generic
			boost::shared_ptr<DataField> dataField;
			if (v.first == "ASCIIDataField")
			{
				dataField.reset(new ASCIIDataField());
			}
			else if (v.first == "BinaryDataField")
			{
				dataField.reset(new BinaryDataField());
			}
			else if (v.first == "NumberDataField")
			{
				dataField.reset(new NumberDataField());
			}
			else if (v.first == "ParityDataField")
			{
				dataField.reset(new ParityDataField());
			}

			if (dataField)
			{
				boost::property_tree::ptree f = v.second;
				dataField->unSerialize(f);
				d_fieldList.push_back(dataField);
			}
		}
	}

	std::string CustomFormat::getDefaultXmlNodeName() const
	{
		return "CustomFormat";
	}

	bool CustomFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<CustomFormat> pFormat = boost::dynamic_pointer_cast<CustomFormat>(format);
			if (pFormat && pFormat->getDataLength() == getDataLength())
			{
				std::list<boost::shared_ptr<DataField> > fields = pFormat->getFieldList();
				if (fields.size() == d_fieldList.size())
				{
					ret = true;
					for(std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(), fi = fields.begin(); ret && i != d_fieldList.end() && fi != fields.end(); ++i, ++fi)
					{
						ret = (*i)->checkSkeleton(*fi);
					}
				}
			}
		}
		return ret;
	}

	FormatType CustomFormat::getType() const
	{
		return FT_CUSTOM;
	}

	size_t CustomFormat::getSkeletonLinearData(void* data, size_t dataLengthBytes) const
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		std::string xmlstr = const_cast<XmlSerializable*>(dynamic_cast<const XmlSerializable*>(this))->serialize();
		std::vector<unsigned char> xmlbuf(xmlstr.begin(), xmlstr.end());

		if (data != NULL)
		{
			if (dataLengthBytes < xmlbuf.size())
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "The buffer size is too short.");
			}
			memcpy(data, &xmlbuf[0], xmlbuf.size());
		}

		return (xmlbuf.size() * 8);
	}

	void CustomFormat::setSkeletonLinearData(const void* data, size_t dataLengthBytes)
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		std::vector<unsigned char> xmlbuf((unsigned char*)data, (unsigned char*)data + dataLengthBytes);
		std::string xmlstr = BufferHelper::getStdString(xmlbuf);
		dynamic_cast<XmlSerializable*>(this)->unSerialize(xmlstr, "");
	}

	boost::shared_ptr<DataField> CustomFormat::getFieldForPosition(unsigned int position)
	{
		boost::shared_ptr<DataField> field;
		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); !field && i != d_fieldList.end(); ++i)
		{
			if (position >= (*i)->getPosition()  && position < ((*i)->getPosition() + (*i)->getDataLength()))
			{
				field = (*i);
			}
		}

		return field;
	}
}
