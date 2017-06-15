/**
 * \file getronik40bitformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Getronik 40-Bit Format.
 */

#include <boost/property_tree/ptree.hpp>
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

        std::shared_ptr<NumberDataField> field(new NumberDataField());
        field->setName("Uid");
        field->setIsIdentifier(true);
        field->setDataLength(16);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
        field = std::shared_ptr<NumberDataField>(new NumberDataField());
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
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Field"));
        return static_cast<unsigned short int>(field->getValue());
    }

    void Getronik40BitFormat::setField(unsigned short int field)
    {
        std::shared_ptr<NumberDataField> nfield = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("Field"));
        nfield->setValue(field);
        d_formatLinear.d_field = field;
    }

    size_t Getronik40BitFormat::getFormatLinearData(std::vector<uint8_t>& /*data*/) const
    {
        return 0;
    }

    void Getronik40BitFormat::setFormatLinearData(const std::vector<uint8_t>& /*data*/, size_t* /*indexByte*/)
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

    bool Getronik40BitFormat::checkSkeleton(std::shared_ptr<Format> format) const
    {
        bool ret = false;
        if (format)
        {
            std::shared_ptr<Getronik40BitFormat> pFormat = std::dynamic_pointer_cast<Getronik40BitFormat>(format);
            if (pFormat)
            {
                ret = (
                    (d_formatLinear.d_field == 0 || d_formatLinear.d_field == pFormat->getField())
                    );
            }
        }
        return ret;
    }

    unsigned char Getronik40BitFormat::getRightParity(const BitsetStream& data) const
    {
        unsigned char parity = 0x00;

        if (data.getByteSize() != 0)
        {
            parity = calculateParity(data, PT_EVEN, static_cast<size_t>(0), 38);
            size_t pos = 39;
            parity = parity ^ ((data.getData()[pos / 8] << (pos % 8)) >> 7);
        }

        return parity;
    }

	std::vector<uint8_t> Getronik40BitFormat::getLinearData() const
    {
		BitsetStream data;

		data.append(0x2E);

		convertField(data, getUid(), 16);
		convertField(data, getField(), 14);

		data.writeAt(39, 0x01, 7, 1);
		data.writeAt(38, getRightParity(data), 7, 1);
		
		return data.getData();
    }

    void Getronik40BitFormat::setLinearData(const std::vector<uint8_t>& data)
    {
        unsigned int pos = 0;
        BitsetStream fixedValue;
		BitsetStream _data;

		_data.concat(data);

        if (_data.getByteSize() != 0)
        {
            if (_data.getByteSize() * 8 < getDataLength())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
            }

            BitHelper::extract(fixedValue, _data, pos, 8);
            if (fixedValue.getData()[0] != 0x2E)
            {
                char exceptionmsg[256];
                sprintf(exceptionmsg, "Getronik 40-Bit: fixed left value doesn't match (%x != %x).", fixedValue, 0x2E);
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
            }
            pos = 39;
            fixedValue.writeAt(0, 0x00);
            BitHelper::extract(fixedValue, _data, pos, 1);
            fixedValue.writeAt(0, (fixedValue.getData()[0] >> 7) & 0x01);
            if (!fixedValue.test(7))
            {
                char exceptionmsg[256];
                sprintf(exceptionmsg, "Getronik 40-Bit: fixed right value doesn't match (%x != %x).", fixedValue, 0x01);
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
            }
            pos = 8;
            d_uid = revertField(_data, &pos, 16);
            d_formatLinear.d_field = (unsigned short)revertField(_data, &pos, 14);
            pos = 38;
            unsigned char parity = getRightParity(_data);
            if (((_data.getData()[pos / 8] << (pos % 8)) >> 7) != parity)
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Right parity format error.");
            }
        }
    }
}