/**
 * \file DataClockFormat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data clock Format.
 */

#include "logicalaccess/Services/AccessControl/Formats/DataClockFormat.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"
#include "logicalaccess/Services/AccessControl/Encodings/BCDNibbleDataType.h"
#include "logicalaccess/Services/AccessControl/Encodings/LittleEndianDataRepresentation.h"

#include "logicalaccess/Services/AccessControl/Formats/CustomFormat/NumberDataField.h"

namespace LOGICALACCESS
{
	DataClockFormat::DataClockFormat()
		: StaticFormat()
	{
		d_dataType.reset(new BCDNibbleDataType());
		d_dataRepresentation.reset(new LittleEndianDataRepresentation());

		boost::shared_ptr<NumberDataField> uidField(new NumberDataField());
		uidField->setName("Uid");
		uidField->setIsIdentifier(true);
		uidField->setDataLength(32);		
		uidField->setDataRepresentation(d_dataRepresentation);
		uidField->setDataType(d_dataType);
		d_fieldList.push_back(uidField);
	}

	DataClockFormat::~DataClockFormat()
	{
	}

	unsigned int DataClockFormat::getDataLength() const
	{
		return 36;
	}

	string DataClockFormat::getName() const
	{
		return string("Data clock");
	}

	size_t DataClockFormat::getFormatLinearData(void* /*data*/, size_t /*dataLengthBytes*/) const
	{
		return 0;
	}

	void DataClockFormat::setFormatLinearData(const void* /*data*/, size_t* /*indexByte*/)
	{
		//DOES NOTHING
	}

	FormatType DataClockFormat::getType() const
	{
		return FT_DATACLOCK;
	}

	void DataClockFormat::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getType());
		node.put("Uid", getUid());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void DataClockFormat::unSerialize(boost::property_tree::ptree& node)
	{
		setUid(node.get_child("Uid").get_value<unsigned long long>());
	}

	std::string DataClockFormat::getDefaultXmlNodeName() const
	{
		return "DataClockFormat";
	}

	bool DataClockFormat::checkSkeleton(boost::shared_ptr<Format> format) const
	{
		bool ret = false;
		if (format)
		{
			boost::shared_ptr<DataClockFormat> pFormat = boost::dynamic_pointer_cast<DataClockFormat>(format);
			if (pFormat)
			{
				ret = true;
			}
		}
		return ret;
	}

	void DataClockFormat::getLinearData(void* data, size_t dataLengthBytes) const
	{
		unsigned int pos = 0;
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasWriteFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		convertField(data, dataLengthBytes, &pos, getUid(), 32);
		
		if (data != NULL)
		{
			pos = 32;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity1(data, dataLengthBytes), 7, 1);
			pos = 33;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity2(data, dataLengthBytes), 7, 1);
			pos = 34;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity3(data, dataLengthBytes), 7, 1);
			pos = 35;
			BitHelper::writeToBit(data, dataLengthBytes, &pos, getRightParity4(data, dataLengthBytes), 7, 1);
		}
	}

	void DataClockFormat::setLinearData(const void* data, size_t dataLengthBytes)
	{
#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		unsigned int pos = 0;
		setUid(revertField(data, dataLengthBytes, &pos, 32));
		
		if (data != NULL)
		{
			pos = 32;
			unsigned char parity = getRightParity1(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Right parity 1 format error.");
			}

			pos = 33;
			parity = getRightParity2(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Right parity 2 format error.");
			}

			pos = 34;
			parity = getRightParity3(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Right parity 3 format error.");
			}

			pos = 35;
			parity = getRightParity4(data, dataLengthBytes);
			if ((unsigned char)((unsigned char)(reinterpret_cast<const unsigned char*>(data)[pos / 8] << (pos % 8)) >> 7) != parity)
			{
				THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Right parity 4 format error.");
			}
		}
	}

	unsigned char DataClockFormat::getRightParity(const void* data, size_t dataLengthBytes, ParityType type, unsigned int* positions, size_t positionLength) const
	{
		unsigned char parity = 0x00;

		if (data != NULL)
		{
			parity = Format::calculateParity(data, dataLengthBytes, type, positions, positionLength);
		}

		return parity;
	}

	unsigned char DataClockFormat::getRightParity1(const void* data, size_t dataLengthBytes) const
	{
		unsigned int positions[8];
		positions[0] = 0;
		positions[1] = 4;
		positions[2] = 8;
		positions[3] = 12;
		positions[4] = 16;
		positions[5] = 20;
		positions[6] = 24;
		positions[7] = 28;

		return getRightParity(data, dataLengthBytes, PT_EVEN, positions, 8);
	}

	unsigned char DataClockFormat::getRightParity2(const void* data, size_t dataLengthBytes) const
	{
		unsigned int positions[8];
		positions[0] = 1;
		positions[1] = 5;
		positions[2] = 9;
		positions[3] = 13;
		positions[4] = 17;
		positions[5] = 21;
		positions[6] = 25;
		positions[7] = 29;

		return getRightParity(data, dataLengthBytes, PT_ODD, positions, 8);
	}

	unsigned char DataClockFormat::getRightParity3(const void* data, size_t dataLengthBytes) const
	{
		unsigned int positions[8];
		positions[0] = 2;
		positions[1] = 6;
		positions[2] = 10;
		positions[3] = 14;
		positions[4] = 18;
		positions[5] = 22;
		positions[6] = 26;
		positions[7] = 30;

		return getRightParity(data, dataLengthBytes, PT_EVEN, positions, 8);
	}

	unsigned char DataClockFormat::getRightParity4(const void* data, size_t dataLengthBytes) const
	{
		unsigned int positions[8];
		positions[0] = 3;
		positions[1] = 7;
		positions[2] = 11;
		positions[3] = 15;
		positions[4] = 19;
		positions[5] = 23;
		positions[6] = 27;
		positions[7] = 31;

		return getRightParity(data, dataLengthBytes, PT_EVEN, positions, 8);
	}
}

