/**
 * \file customformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Custom Format.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/paritydatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/tlvdatafield.hpp>
#include <logicalaccess/bufferhelper.hpp>

#include <cstring>
#include <stdlib.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
CustomFormat::CustomFormat()
    : Format()
    , d_name("Custom")
{
}

CustomFormat::~CustomFormat()
{
}

unsigned int CustomFormat::getDataLength() const
{
    unsigned int dataLength = 0;
    for (std::list<std::shared_ptr<DataField>>::const_iterator i = d_fieldList.cbegin();
         i != d_fieldList.cend(); ++i)
    {
        unsigned int maxLength = (*i)->getPosition() + (*i)->getDataLength();
        if (maxLength > dataLength)
        {
            dataLength = maxLength;
        }
    }

    return dataLength;
}

void CustomFormat::setName(const std::string &name)
{
    d_name = name;
}

std::string CustomFormat::getName() const
{
    return d_name;
}

ByteVector CustomFormat::getLinearData() const
{
    BitsetStream data(getDataLength());
    std::list<std::shared_ptr<DataField>> sortedFieldList = d_fieldList;
    sortedFieldList.sort(FieldSortPredicate);
    for (std::list<std::shared_ptr<DataField>>::const_iterator i =
             sortedFieldList.begin();
         i != sortedFieldList.end(); ++i)
    {
        auto tmp         = (*i)->getLinearData(data);
        unsigned int pos = (*i)->getPosition();
        data.writeAt(pos, tmp.getData(), 0, tmp.getBitSize());
    }
    return data.getData();
}

void CustomFormat::setLinearData(const ByteVector &data)
{
    std::list<std::shared_ptr<DataField>> sortedFieldList = d_fieldList;
    sortedFieldList.sort(FieldSortPredicate);
    for (std::list<std::shared_ptr<DataField>>::iterator i = sortedFieldList.begin();
         i != sortedFieldList.end(); ++i)
    {
        (*i)->setLinearData(data);
    }
}

void CustomFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("Name", d_name);

    boost::property_tree::ptree fnode;
    for (std::list<std::shared_ptr<DataField>>::const_iterator i = d_fieldList.cbegin();
         i != d_fieldList.cend(); ++i)
    {
        (*i)->serialize(fnode);
    }
    node.add_child("Fields", fnode);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void CustomFormat::unSerialize(boost::property_tree::ptree &node)
{
    d_fieldList.clear();
    d_name = node.get_child("Name").get_value<std::string>();
    BOOST_FOREACH (boost::property_tree::ptree::value_type const &v,
                   node.get_child("Fields"))
    {
        // TODO: improve the unserialization process to be generic
        std::shared_ptr<DataField> dataField;
        if (v.first == "StringDataField")
        {
            dataField.reset(new StringDataField());
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
		else if (v.first == "TLVDataField")
		{
			dataField.reset(new TLVDataField());
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

bool CustomFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<CustomFormat> pFormat =
            std::dynamic_pointer_cast<CustomFormat>(format);
        if (pFormat && pFormat->getDataLength() == getDataLength())
        {
            std::vector<std::shared_ptr<DataField>> fieldsvector =
                pFormat->getFieldList();
            auto fields = std::list<std::shared_ptr<DataField>>(fieldsvector.begin(),
                                                                fieldsvector.end());
            if (fields.size() == d_fieldList.size())
            {
                ret = true;
                for (std::list<std::shared_ptr<DataField>>::const_iterator
                         i  = d_fieldList.cbegin(),
                         fi = fields.cbegin();
                     ret && i != d_fieldList.cend() && fi != fields.cend(); ++i, ++fi)
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

size_t CustomFormat::getSkeletonLinearData(ByteVector &data) const
{

  XmlSerializable *tmp = const_cast<XmlSerializable *>(dynamic_cast<const XmlSerializable *>(this));
  if (tmp == nullptr)
    return 0;
  std::string xmlstr = tmp->serialize();
    ByteVector xmlbuf(xmlstr.begin(), xmlstr.end());

    if (data.size() != 0)
    {
        if (data.size() < xmlbuf.size())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "The buffer size is too short.");
        }
        std::copy(&xmlbuf[0], &xmlbuf[0] + xmlbuf.size(), data.begin());
    }

    return (xmlbuf.size() * 8);
}

void CustomFormat::setSkeletonLinearData(const ByteVector &data)
{
    std::string xmlstr = BufferHelper::getStdString(data);
    XmlSerializable *tmp = dynamic_cast<XmlSerializable *>(this);
    if (tmp != nullptr)
      tmp->unSerialize(xmlstr, "");
}

std::shared_ptr<DataField> CustomFormat::getFieldForPosition(unsigned int position) const
{
    std::shared_ptr<DataField> field;
    for (std::list<std::shared_ptr<DataField>>::const_iterator i = d_fieldList.cbegin();
         !field && i != d_fieldList.cend(); ++i)
    {
        if (position >= (*i)->getPosition() &&
            position < ((*i)->getPosition() + (*i)->getDataLength()))
        {
            field = (*i);
        }
    }

    return field;
}
}
