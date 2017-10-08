/**
 * \file wiegand37format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>
 * \brief Wiegand 37 Format.
 */

#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
    Wiegand37Format::Wiegand37Format()
        : WiegandFormat()
    {
        d_leftParityType = PT_EVEN;
        d_leftParityLength = 18;
        d_rightParityType = PT_ODD;
        d_rightParityLength = 18;

        std::shared_ptr<NumberDataField> field(new NumberDataField());
        field->setName("Uid");
        field->setIsIdentifier(true);
        field->setDataLength(35);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
    }

    Wiegand37Format::~Wiegand37Format()
    {
    }

    unsigned int Wiegand37Format::getDataLength() const
    {
        return 37;
    }

	std::string Wiegand37Format::getName() const
    {
        return std::string("Wiegand 37");
    }

    void Wiegand37Format::getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const
    {
        unsigned int pos = 1;

        convertField(data, dataLengthBytes, &pos, getUid(), 35);
    }

    void Wiegand37Format::setLinearDataWithoutParity(const void* data, size_t dataLengthBytes)
    {
        unsigned int pos = 1;

        setUid(revertField(data, dataLengthBytes, &pos, 35));
    }

    size_t Wiegand37Format::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
    {
        return 0;
    }

    void Wiegand37Format::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
    {
    }

    FormatType Wiegand37Format::getType() const
    {
        return FT_WIEGAND37;
    }

    void Wiegand37Format::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getType());
        node.put("Uid", getUid());

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void Wiegand37Format::unSerialize(boost::property_tree::ptree& node)
    {
        setUid(node.get_child("Uid").get_value<unsigned long long>());
    }

	std::string Wiegand37Format::getDefaultXmlNodeName() const
    {
        return "Wiegand37Format";
    }

    bool Wiegand37Format::checkSkeleton(std::shared_ptr<Format> format) const
    {
        bool ret = false;
        if (format)
        {
            std::shared_ptr<Wiegand37Format> pFormat = std::dynamic_pointer_cast<Wiegand37Format>(format);
            if (pFormat)
            {
                ret = true;
            }
        }
        return ret;
    }
}