/**
 * \file format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Format Base.
 */

#include "logicalaccess/services/accesscontrol/formats/format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/corporate1000format.hpp"
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/hidhoneywellformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/getronik40bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bariumferritepcscformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/rawformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/paritydatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    Format::Format()
    {
    }

    bool FieldSortPredicate(const std::shared_ptr<DataField>& lhs, const std::shared_ptr<DataField>& rhs)
    {
        bool before;
        if (std::dynamic_pointer_cast<ParityDataField>(lhs))
        {
            if (std::dynamic_pointer_cast<ParityDataField>(rhs))
            {
                if (std::dynamic_pointer_cast<ParityDataField>(rhs)->checkFieldDependecy(lhs))
                {
                    before = true;
                }
                else
                {
                    if (std::dynamic_pointer_cast<ParityDataField>(lhs)->checkFieldDependecy(rhs))
                    {
                        before = false;
                    }
                    else
                    {
                        before = lhs->getPosition() < rhs->getPosition();
                    }
                }
            }
            else
            {
                before = false;
            }
        }
        else
        {
            if (std::dynamic_pointer_cast<ParityDataField>(rhs))
            {
                before = true;
            }
            else
            {
                before = lhs->getPosition() < rhs->getPosition();
            }
        }
        return before;
    }

    unsigned char Format::calculateParity(const BitsetStream& data, ParityType parityType,
											unsigned int* positions, size_t nbPositions)
    {
        unsigned char parity = 0x00;
        for (size_t i = 0; i < nbPositions && i < (data.getByteSize() * 8); i++)
        {
            parity = (unsigned char)((parity & 0x01) ^ ((data.getData()[positions[i] / 8] >> (7 - (positions[i] % 8))) & 0x01));
        }

        switch (parityType)
        {
        case PT_EVEN:
            break;

        case PT_ODD:
            parity = (unsigned char)((~parity) & 0x01);
            break;

        case PT_NONE:
            parity = 0x00;
            break;
        }

        return parity;
    }

    std::shared_ptr<Format> Format::getByFormatType(FormatType type)
    {
        std::shared_ptr<Format> ret;
        switch (type)
        {
        case FT_WIEGAND26:
            ret.reset(new Wiegand26Format());
            break;

        case FT_WIEGAND34:
            ret.reset(new Wiegand34Format());
            break;

        case FT_WIEGAND34FACILITY:
            ret.reset(new Wiegand34WithFacilityFormat());
            break;

        case FT_WIEGAND37:
            ret.reset(new Wiegand37Format());
            break;

        case FT_WIEGAND37FACILITY:
            ret.reset(new Wiegand37WithFacilityFormat());
            break;

        case FT_CORPORATE1000:
            ret.reset(new Corporate1000Format());
            break;

        case FT_DATACLOCK:
            ret.reset(new DataClockFormat());
            break;

        case FT_FASCN200BIT:
            ret.reset(new FASCN200BitFormat());
            break;

        case FT_HIDHONEYWELL:
            ret.reset(new HIDHoneywellFormat());
            break;

        case FT_GETRONIK40BIT:
            ret.reset(new Getronik40BitFormat());
            break;

        case FT_BARIUM_FERRITE_PCSC:
            ret.reset(new BariumFerritePCSCFormat());
            break;

        case FT_RAW:
            ret.reset(new RawFormat());
            break;

        case FT_CUSTOM:
            ret.reset(new CustomFormat());
            break;

        default:
            break;
        }

        return ret;
    }

    std::vector<unsigned char> Format::getIdentifier()
    {
        std::vector<unsigned char> ret;

        std::list<std::shared_ptr<DataField> > fields = getFieldList();
        for (std::list<std::shared_ptr<DataField> >::iterator i = fields.begin(); i != fields.end(); ++i)
        {
            std::shared_ptr<ValueDataField> vfield = std::dynamic_pointer_cast<ValueDataField>(*i);
            if (vfield)
            {
                if (vfield->getIsIdentifier())
                {
					if (std::dynamic_pointer_cast<StringDataField>(vfield))
                    {
						BufferHelper::setString(ret, std::dynamic_pointer_cast<StringDataField>(vfield)->getValue());
                    }
                    else if (std::dynamic_pointer_cast<BinaryDataField>(vfield))
                    {
                        std::vector<unsigned char> bindata = std::dynamic_pointer_cast<BinaryDataField>(vfield)->getValue();
                        ret.insert(ret.end(), bindata.begin(), bindata.end());
                    }
                    else if (std::dynamic_pointer_cast<NumberDataField>(vfield))
                    {
                        BufferHelper::setUInt64(ret, std::dynamic_pointer_cast<NumberDataField>(vfield)->getValue());
                    }
                }
            }
        }

        if (ret.size() < 1)
        {
            size_t dataLengthByte = (getDataLength() + 7) / 8;
            ret.resize(dataLengthByte);

            ret = getLinearData();
        }

        return ret;
    }

    std::vector<std::string> Format::getValuesFieldList() const
    {
        std::vector<std::string> fields;
        for (std::list<std::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); i != d_fieldList.end(); ++i)
        {
            std::shared_ptr<ValueDataField> vfield = std::dynamic_pointer_cast<ValueDataField>(*i);
            if (vfield)
            {
                if (!vfield->getIsFixedField())
                {
                    fields.push_back((*i)->getName());
                }
            }
        }
        return fields;
    }

    unsigned int Format::getFieldLength(const string& field) const
    {
        unsigned int length = 0;

        for (std::list<std::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); length == 0 && i != d_fieldList.end(); ++i)
        {
            if ((*i)->getName() == field)
            {
                length = (*i)->getDataLength();
            }
        }

        return length;
    }

    std::shared_ptr<DataField> Format::getFieldFromName(std::string field) const
    {
        std::shared_ptr<DataField> ret;

        for (std::list<std::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); !ret && i != d_fieldList.end(); ++i)
        {
            if ((*i)->getName() == field)
            {
                ret = (*i);
            }
        }

        return ret;
    }

    std::list<std::shared_ptr<DataField> > Format::getFieldList()
    {
        d_fieldList.sort(FieldSortPredicate);
        return d_fieldList;
    }

    void Format::setFieldList(std::list<std::shared_ptr<DataField> > fields)
    {
        d_fieldList = fields;
    }
}