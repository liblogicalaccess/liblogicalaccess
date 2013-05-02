/**
 * \file GenericTagAccessControlCardService.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief GenericTag Access Control Card service.
 */

#include "GenericTagAccessControlCardService.h"
#include "GenericTagCardProvider.h"
#include "GenericTagChip.h"
#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "logicalaccess/Services/AccessControl/Formats/BitHelper.h"


#ifdef __linux__
#include <cstring>
#endif

namespace LOGICALACCESS
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

#ifdef _LICENSE_SYSTEM
		if (!d_license.hasReadFormatAccess())
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_LICENSENOACCESS);
		}
#endif

		boost::shared_ptr<Format> formatret;
		if (format)
		{
			formatret = LOGICALACCESS::Format::getByFormatType(format->getType());
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
					if (getCardProvider()->readData(boost::shared_ptr<Location>(), boost::shared_ptr<AccessInfo>(), databuf, length, CB_DEFAULT))
					{
						unsigned int realDataLengthBits = boost::dynamic_pointer_cast<GenericTagChip>(getCardProvider()->getChip())->getTagIdBitsLength();
						if (realDataLengthBits == 0)
						{
							realDataLengthBits = length;
						}

						if (realDataLengthBits >= formatret->getDataLength())
						{
							unsigned int writePosBit = 0;
							BitHelper::writeToBit(formatBuf, formatlength, &writePosBit, databuf, length, dataLengthBits, dataLengthBits - realDataLengthBits, realDataLengthBits);
							formatret->setLinearData(formatBuf, formatlength);
							ret = true;
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

