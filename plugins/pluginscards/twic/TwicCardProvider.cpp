/**
 * \file twiccardprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic card profiles.
 */

#include "twiccardprovider.hpp"
#include "twicaccesscontrolcardservice.hpp"


namespace logicalaccess
{
	TwicCardProvider::TwicCardProvider()
	{
		
	}

	TwicCardProvider::~TwicCardProvider()
	{
		
	}

	size_t TwicCardProvider::getMinimumBytesRepresentation(size_t value)
	{
		size_t encodedlength = 0;

		// Will works only in LITTLE ENDIAN representation.
		for (size_t i = sizeof(value)-1; i > 0 && encodedlength == 0; --i)
		{
			if (reinterpret_cast<unsigned char*>(&value)[i] != 0x00)
			{
				encodedlength = i;
			}
		}

		return (encodedlength + 1);
	}

	size_t TwicCardProvider::getValueFromBytes(unsigned char* data, size_t datalength)
	{
		size_t value = 0;
		for (size_t i = 0, n = datalength-1; i < datalength; i++, n--)
		{
			value |= (data[i] << (n * 8));
		}
		return value;
	}

	size_t TwicCardProvider::getMaximumDataObjectLength(int64_t dataObject)
	{
		size_t maxlen = 0;

		switch (dataObject)
		{
		case 0x5FC104:
			maxlen = 57;
			break;

		case 0xDFC101:
			maxlen = 40;
			break;

		case 0x5FC102:
			maxlen = 2461;
			break;

		case 0xDFC103:
			maxlen = 2504;
			break;

		case 0xDFC10F:
			maxlen = 920;
			break;
		}

		return maxlen;
	}

	size_t TwicCardProvider::getMaximumTagLength(int64_t dataObject, unsigned char tag)
	{
		size_t maxlen = 0;

		switch (dataObject)
		{
		case 0x5FC104:
			switch (tag)
			{
			case 0x30:
				maxlen = 25;
				break;

			case 0x34:
				maxlen = 16;
				break;

			case 0x35:
				maxlen = 8;
				break;

			case 0xFE:
				maxlen = 0;
				break;
			}
			break;

		case 0xDFC101:
			switch (tag)
			{
			case 0xC0:
				maxlen = 32;
				break;

			case 0xC1:
				maxlen = 1;
				break;

			case 0xC2:
				maxlen = 1;
				break;
			}
			break;

		case 0x5FC102:
			switch (tag)
			{
			case 0x30:
				maxlen = 25;
				break;

			case 0x34:
				maxlen = 16;
				break;

			case 0x35:
				maxlen = 8;
				break;

			case 0x3E:
				maxlen = 2400;
				break;

			case 0xFE:
				maxlen = 0;
				break;
			}
			break;

		case 0xDFC103:
			switch (tag)
			{
			case 0xBC:
				maxlen = 2500;
				break;
			}
			break;

		case 0xDFC10F:
			switch (tag)
			{
			case 0xBA:
				maxlen = 12;
				break;

			case 0xBB:
				maxlen = 900;
				break;

			case 0xFE:
				maxlen = 0;
				break;
			}
			break;
		}

		return maxlen;
	}

	size_t TwicCardProvider::getDataObjectLength(int64_t dataObject, bool withObjectLength)
	{
		size_t maxlen = getMaximumDataObjectLength(dataObject);
		size_t length = 0;
		size_t encodedlength = getMinimumBytesRepresentation(maxlen);
		if (encodedlength > 0)
		{
			size_t datalength = encodedlength + 1;
			unsigned char* data = new unsigned char[datalength];
			try
			{
				if (getTwicCommands()->getTWICData(data, datalength, dataObject))
				{
					length = getValueFromBytes(&data[1], datalength-1);
				}
			}
			catch(std::exception&)
			{
				delete[] data;
				return 0;
			}
			delete[] data;
		}

		if (withObjectLength)
		{
			length += 1 + encodedlength;
		}

		return length;
	}	

	bool TwicCardProvider::getTagData(boost::shared_ptr<TwicLocation> location, void* data, size_t datalen, void* datatag, size_t& datataglen)
	{
		bool ret = false;
		bool processed = false;
		unsigned char* ucdata = reinterpret_cast<unsigned char*>(data);
		size_t pos = 0;
		while (pos < datalen && !processed)
		{
			unsigned char currentTag = ucdata[pos++];
			size_t encodedlength = getMinimumBytesRepresentation(getMaximumTagLength(location->dataObject, location->tag));
			size_t currentTagLength = getValueFromBytes(&ucdata[pos], encodedlength);
			pos += encodedlength;
			if (currentTag == location->tag)
			{
				processed = true;
				if (datatag != NULL)
				{
					if (datataglen >= currentTagLength)
					{
						memcpy(datatag, &ucdata[pos], datataglen);
						ret = true;
					}
				}
				else
				{
					datataglen = currentTagLength;
				}
				pos += currentTagLength;
			}
		}

		return ret;
	}

	bool TwicCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior behaviorFlags)
	{
		bool ret = false;

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadDataAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<ISO7816Location> icISOLocation = boost::dynamic_pointer_cast<ISO7816Location>(location);
		boost::shared_ptr<TwicLocation> icLocation = boost::dynamic_pointer_cast<TwicLocation>(location);

		if (icISOLocation)
		{
			return ISO7816CardProvider::readData(location, aiToUse, data, dataLength, behaviorFlags);
		}
		EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a TwicLocation or ISO7816Location.");

		getTwicCommands()->selectTWICApplication();
		if (icLocation->tag == 0x00)
		{
			size_t dataObjectLength = dataLength;			
			if (getTwicCommands()->getTWICData(data, dataObjectLength, icLocation->dataObject) && dataObjectLength == dataLength)
			{
				ret = true;
			}
		}
		else
		{
			// A tag is specified, the user want to get only the tag's data.
			size_t dataObjectLength = getDataObjectLength(icLocation->dataObject, true);
			unsigned char* fulldata = new unsigned char[dataObjectLength];

			if (getTwicCommands()->getTWICData(fulldata, dataObjectLength, icLocation->dataObject))
			{
				size_t offset = getMinimumBytesRepresentation(getMaximumDataObjectLength(icLocation->dataObject)) + 1;
				if (offset < dataObjectLength)
				{
					if (getTagData(icLocation, &fulldata[offset], dataObjectLength - offset, data, dataLength))
					{
						ret = true;
					}
				}
			}

			delete[] fulldata;
		}

		return ret;
	}

	boost::shared_ptr<CardService> TwicCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new TwicAccessControlCardService(shared_from_this()));
			}
			break;
		case CST_NFC_TAG:
		  break;
		}

		if (!service)
		{
			service = ISO7816CardProvider::getService(serviceType);
		}

		return service;
	}
}
