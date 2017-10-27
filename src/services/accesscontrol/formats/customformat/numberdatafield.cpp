/**
 * \file numberdatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Number Data field.
 */

#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
NumberDataField::NumberDataField()
    : ValueDataField()
{
    d_length = 0;
    d_value  = 0;
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

BitsetStream NumberDataField::getLinearData(const BitsetStream &data) const
{
    BitsetStream dataTmp;
    convertNumericData(dataTmp, d_value, d_length);

    return dataTmp;
}

void NumberDataField::setLinearData(const ByteVector &data)
{
    BitsetStream _data;

    _data.concat(data);
    d_value = revertNumericData(_data, getPosition(), d_length);
}

bool NumberDataField::checkSkeleton(std::shared_ptr<DataField> field) const
{
    bool ret = false;
    if (field)
    {
        std::shared_ptr<NumberDataField> pField =
            std::dynamic_pointer_cast<NumberDataField>(field);
        if (pField)
        {
            ret = (pField->getDataLength() == getDataLength() &&
                   pField->getPosition() == getPosition() &&
                   (pField->getIsFixedField() || getValue() == 0 ||
                    pField->getValue() == getValue()));
        }
    }

    return ret;
}

void NumberDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    ValueDataField::serialize(node);
    node.put("Value", d_value);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void NumberDataField::unSerialize(boost::property_tree::ptree &node)
{
    ValueDataField::unSerialize(node);
    d_value = node.get_child("Value").get_value<long long>();
}

std::string NumberDataField::getDefaultXmlNodeName() const
{
    return "NumberDataField";
}
}