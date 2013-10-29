/**
 * \file customformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Custom Format.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/asciidatafield.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/paritydatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <cstring>
#include <stdlib.h>

#include <boost/foreach.hpp>

namespace logicalaccess
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
		for(std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.cbegin(); i != d_fieldList.cend(); ++i)
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
		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.cbegin(); i != d_fieldList.cend(); ++i)
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
					for(std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.cbegin(), fi = fields.cbegin(); ret && i != d_fieldList.cend() && fi != fields.cend(); ++i, ++fi)
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
		std::string xmlstr = const_cast<XmlSerializable*>(dynamic_cast<const XmlSerializable*>(this))->serialize();
		std::vector<unsigned char> xmlbuf(xmlstr.begin(), xmlstr.end());

		if (data != NULL)
		{
			if (dataLengthBytes < xmlbuf.size())
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The buffer size is too short.");
			}
			memcpy(data, &xmlbuf[0], xmlbuf.size());
		}

		return (xmlbuf.size() * 8);
	}

	void CustomFormat::setSkeletonLinearData(const void* data, size_t dataLengthBytes)
	{
		std::vector<unsigned char> xmlbuf((unsigned char*)data, (unsigned char*)data + dataLengthBytes);
		std::string xmlstr = BufferHelper::getStdString(xmlbuf);
		dynamic_cast<XmlSerializable*>(this)->unSerialize(xmlstr, "");
	}

	boost::shared_ptr<DataField> CustomFormat::getFieldForPosition(unsigned int position)
	{
		boost::shared_ptr<DataField> field;
		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.cbegin(); !field && i != d_fieldList.cend(); ++i)
		{
			if (position >= (*i)->getPosition()  && position < ((*i)->getPosition() + (*i)->getDataLength()))
			{
				field = (*i);
			}
		}

		return field;
	}
}
