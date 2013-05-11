/**
 * \file Format.cpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Format Base.
 */

#include "logicalaccess/Services/AccessControl/Formats/Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand26Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand34WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37Format.h"
#include "logicalaccess/Services/AccessControl/Formats/Wiegand37WithFacilityFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Corporate1000Format.h"
#include "logicalaccess/Services/AccessControl/Formats/DataClockFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/FASCN200BitFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/HIDHoneywellFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/Getronik40BitFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BariumFerritePCSCFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/RawFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/CustomFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/ASCIIDataField.h"
#include "logicalaccess/Services/AccessControl/Encodings/BigEndianDataRepresentation.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"
#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/ParityDataField.h"
#include "logicalaccess/Services/AccessControl/Formats//CustomFormat/BinaryDataField.h"
#include "logicalaccess/BufferHelper.h"


namespace logicalaccess
{
	Format::Format()
	{
	}

	bool FieldSortPredicate(const boost::shared_ptr<DataField>& lhs, const boost::shared_ptr<DataField>& rhs)
	{
		bool before;
		if (boost::dynamic_pointer_cast<ParityDataField>(lhs))
		{
			if (boost::dynamic_pointer_cast<ParityDataField>(rhs))
			{
				if (boost::dynamic_pointer_cast<ParityDataField>(rhs)->checkFieldDependecy(lhs))
				{
					before = true;
				}
				else
				{
					if (boost::dynamic_pointer_cast<ParityDataField>(lhs)->checkFieldDependecy(rhs))
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
			if (boost::dynamic_pointer_cast<ParityDataField>(rhs))
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

	unsigned char Format::calculateParity(const void* data, size_t dataLengthBytes, ParityType parityType, unsigned int* positions, size_t nbPositions)
	{
		unsigned char parity = 0x00;
		for (size_t i = 0; i < nbPositions && i < (dataLengthBytes * 8); i++)
		{
			parity = (unsigned char)((parity & 0x01) ^ ((unsigned char)(reinterpret_cast<const char*>(data)[positions[i] / 8] >> (7 - (positions[i] % 8))) & 0x01));
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
	
	boost::shared_ptr<Format> Format::getByFormatType(FormatType type)
	{
		boost::shared_ptr<Format> ret;
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

		std::list<boost::shared_ptr<DataField> > fields = getFieldList();
		for (std::list<boost::shared_ptr<DataField> >::iterator i = fields.begin(); i != fields.end(); ++i)
		{
			boost::shared_ptr<ValueDataField> vfield = boost::dynamic_pointer_cast<ValueDataField>(*i);
			if (vfield)
			{
				if (vfield->getIsIdentifier())
				{
					if (boost::dynamic_pointer_cast<ASCIIDataField>(vfield))
					{
						BufferHelper::setString(ret, boost::dynamic_pointer_cast<ASCIIDataField>(vfield)->getValue());
					}
					else if(boost::dynamic_pointer_cast<BinaryDataField>(vfield))
					{
						std::vector<unsigned char> bindata = boost::dynamic_pointer_cast<BinaryDataField>(vfield)->getValue();
						ret.insert(ret.end(), bindata.begin(), bindata.end());
					}
					else if(boost::dynamic_pointer_cast<NumberDataField>(vfield))
					{
						BufferHelper::setUInt64(ret, boost::dynamic_pointer_cast<NumberDataField>(vfield)->getValue());
					}
				}
			}
		}

		if (ret.size() < 1)
		{
			size_t dataLengthByte = (getDataLength() + 7) / 8;
			ret.resize(dataLengthByte);

			getLinearData(ret.data(), ret.size());
		}

		return ret;
	}

	vector<string> Format::getValuesFieldList() const
	{
		vector<string> fields;
		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); i != d_fieldList.end(); ++i)
		{
			boost::shared_ptr<ValueDataField> vfield = boost::dynamic_pointer_cast<ValueDataField>(*i);
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

		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); length == 0 && i != d_fieldList.end(); ++i)
		{
			if ((*i)->getName() == field)
			{
				length = (*i)->getDataLength();
			}
		}

		return length;
	}

	boost::shared_ptr<DataField> Format::getFieldFromName(std::string field) const
	{
		boost::shared_ptr<DataField> ret;

		for (std::list<boost::shared_ptr<DataField> >::const_iterator i = d_fieldList.begin(); !ret && i != d_fieldList.end(); ++i)
		{
			if ((*i)->getName() == field)
			{
				ret = (*i);
			}
		}

		return ret;
	}

	std::list<boost::shared_ptr<DataField> > Format::getFieldList()
	{
		d_fieldList.sort(FieldSortPredicate);
		return d_fieldList;
	}

	void Format::setFieldList(std::list<boost::shared_ptr<DataField> > fields)
	{
		d_fieldList = fields;
	}
}

