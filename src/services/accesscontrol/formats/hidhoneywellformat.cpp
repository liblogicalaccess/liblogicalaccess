/**
 * \file hidhoneywellformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HID Honeywell Format.
 */

#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/formats/hidhoneywellformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"
#include "logicalaccess/services/accesscontrol/encodings/binarydatatype.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

namespace logicalaccess
{
    HIDHoneywellFormat::HIDHoneywellFormat()
        : StaticFormat()
    {
        d_dataType.reset(new BinaryDataType());
        d_dataRepresentation.reset(new BigEndianDataRepresentation());

        d_formatLinear.d_facilityCode = 0;

        std::shared_ptr<NumberDataField> field(new NumberDataField());
        field->setName("Uid");
        field->setIsIdentifier(true);
        field->setDataLength(16);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
        field = std::shared_ptr<NumberDataField>(new NumberDataField());
        field->setName("FacilityCode");
        field->setDataLength(12);
        field->setDataRepresentation(d_dataRepresentation);
        field->setDataType(d_dataType);
        d_fieldList.push_back(field);
    }

    HIDHoneywellFormat::~HIDHoneywellFormat()
    {
    }

    unsigned int HIDHoneywellFormat::getDataLength() const
    {
        return 40;
    }

    string HIDHoneywellFormat::getName() const
    {
        return string("HID Honeywell 40-Bit");
    }

    unsigned short int HIDHoneywellFormat::getFacilityCode() const
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
        return static_cast<unsigned short int>(field->getValue());
    }

    void HIDHoneywellFormat::setFacilityCode(unsigned short int facilityCode)
    {
        std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("FacilityCode"));
        field->setValue(facilityCode);
        d_formatLinear.d_facilityCode = facilityCode;
    }

    size_t HIDHoneywellFormat::getFormatLinearData(std::vector<uint8_t>& /*data*/) const
    {
        return 0;
    }

    void HIDHoneywellFormat::setFormatLinearData(const std::vector<uint8_t>& /*data*/, size_t* /*indexByte*/)
    {
        //DOES NOTHING
    }

    FormatType HIDHoneywellFormat::getType() const
    {
        return FT_HIDHONEYWELL;
    }

    void HIDHoneywellFormat::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getType());
        node.put("FacilityCode", getFacilityCode());
        node.put("Uid", getUid());

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void HIDHoneywellFormat::unSerialize(boost::property_tree::ptree& node)
    {
        setFacilityCode(node.get_child("FacilityCode").get_value<unsigned short>());
        setUid(node.get_child("Uid").get_value<unsigned short>());
    }

    std::string HIDHoneywellFormat::getDefaultXmlNodeName() const
    {
        return "HIDHoneywellFormat";
    }

    bool HIDHoneywellFormat::checkSkeleton(std::shared_ptr<Format> format) const
    {
        bool ret = false;
        if (format)
        {
            std::shared_ptr<HIDHoneywellFormat> pFormat = std::dynamic_pointer_cast<HIDHoneywellFormat>(format);
            if (pFormat)
            {
                ret = (
                    (d_formatLinear.d_facilityCode == 0 || d_formatLinear.d_facilityCode == pFormat->getFacilityCode())
                    );
            }
        }
        return ret;
    }

    unsigned char HIDHoneywellFormat::getRightParity(const BitsetStream& data, unsigned char rpNo) const
    {
        unsigned char parity = 0x00;

        if (data.getByteSize() != NULL)
        {
            unsigned int positions[4];
            positions[0] = 0 + rpNo;
            positions[1] = 8 + rpNo;
            positions[2] = 16 + rpNo;
            positions[3] = 24 + rpNo;
            parity = Format::calculateParity(data, PT_EVEN, positions, sizeof(positions) / sizeof(int));
        }

        return parity;
    }

	std::vector<uint8_t> HIDHoneywellFormat::getLinearData() const
    {
		BitsetStream data;

		data.append(0x0F, 4, 4);

		convertField(data, getFacilityCode(), 12);
		convertField(data, getUid(), 16);

        for (unsigned char i = 0; i < 8; ++i)
        {
			data.writeAt(32 + i, getRightParity(data, i), 7, 1);
        }
		return data.getData();
    }

    void HIDHoneywellFormat::setLinearData(const std::vector<uint8_t>& data)
    {
        unsigned int pos = 0;
		BitsetStream fixedValue(0x00, 1);
		BitsetStream _data;
		_data.concat(data);

        if (_data.getByteSize() != NULL)
        {
            if (_data.getByteSize() * 8 < getDataLength())
            {
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
            }

            BitHelper::extract(fixedValue, _data, pos, 4);
            pos += 4;
			fixedValue.writeAt(0, (fixedValue.getData()[0] >> 4) & 0x0F);
            if (fixedValue.getData()[0] != 0x0F)
            {
                char exceptionmsg[256];
                sprintf(exceptionmsg, "HID Honeywell 40-Bit: fixed value doesn't match (%x != %x).", fixedValue, 0x0F);
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, exceptionmsg);
            }

            setFacilityCode((unsigned short)revertField(_data, &pos, 12));
            setUid(revertField(_data, &pos, 16));

            for (unsigned char i = 0; i < 8; ++i)
            {
                pos = 32 + i;
                unsigned char parity = getRightParity(_data, i);
                if ((unsigned char)((data[pos / 8] << (pos % 8)) >> 7) != parity)
                {
                    char buftmp[64];
                    sprintf(buftmp, "Right parity %u format error.", i);
                    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, buftmp);
                }
            }
        }
    }
}