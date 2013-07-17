/**
 * \file generictagaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GenericTag Access Control Card service.
 */

#include "generictagaccesscontrolcardservice.hpp"
#include "generictagcardprovider.hpp"
#include "generictagchip.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"


#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
	GenericTagAccessControlCardService::GenericTagAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider)
		: AccessControlCardService(cardProvider)
	{
		
	}

	GenericTagAccessControlCardService::~GenericTagAccessControlCardService()
	{
	}

	boost::shared_ptr<GenericTagCardProvider> GenericTagAccessControlCardService::getGenericTagCardProvider()
	{
		return boost::dynamic_pointer_cast<GenericTagCardProvider>(getCardProvider());
	}

	boost::shared_ptr<Format> GenericTagAccessControlCardService::readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		bool ret = false;

		INFO_SIMPLE_("Try to read a format from Generic Tag...");

		boost::shared_ptr<Format> formatret;
		if (format)
		{
			try
			{
				formatret = Format::getByFormatType(format->getType());
				formatret->unSerialize(format->serialize(), "");
				unsigned int dataLengthBits = static_cast<unsigned int>(getCardProvider()->getChip()->getChipIdentifier().size()) * 8;
	
				if (dataLengthBits > 0)
				{
					unsigned int length = (dataLengthBits + 7) / 8;
					unsigned formatlength = (formatret->getDataLength() + 7) / 8;
					unsigned char *databuf = new unsigned char[length]; 
					unsigned char *formatBuf = new unsigned char[formatlength];				
					memset(databuf, 0x00, length);
					memset(formatBuf, 0x00, formatlength);

					try
					{
						INFO_SIMPLE_("Reading data from Generic Tag...");
						if (getCardProvider()->readData(boost::shared_ptr<Location>(), boost::shared_ptr<AccessInfo>(), databuf, length, CB_DEFAULT))
						{
							unsigned int realDataLengthBits = boost::dynamic_pointer_cast<GenericTagChip>(getCardProvider()->getChip())->getTagIdBitsLength();
							if (realDataLengthBits == 0)
							{
								realDataLengthBits = length * 8;
							}

							if (realDataLengthBits >= formatret->getDataLength())
							{
								INFO_SIMPLE_("Converting data to format...");
								unsigned int writePosBit = 0;
								BitHelper::writeToBit(formatBuf, formatlength, &writePosBit, databuf, length, dataLengthBits, dataLengthBits - realDataLengthBits, realDataLengthBits);
								formatret->setLinearData(formatBuf, formatlength);
								ret = true;
							}
							else
							{
								ERROR_("Cannot read the format: format length (%d) bigger than the total available bits (%d).", formatret->getDataLength(), realDataLengthBits);
							}
						}
					}
					catch(std::exception&)
					{
						delete[] databuf;
						delete[] formatBuf;
						throw;
					}
					delete[] databuf;
					delete[] formatBuf;
				}
			}
			catch(std::exception& ex)
			{
				ERROR_("Error on read format: %s", ex.what());
				throw ex;
			}
		}

		if (!ret)
		{
			formatret.reset();
		}

		return formatret;
	}

	bool GenericTagAccessControlCardService::writeFormat(boost::shared_ptr<Format> /*format*/, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/)
	{
		return false;
	}
}

