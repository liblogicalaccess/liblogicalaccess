/**
 * \file accesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/cards/cardprovider.hpp"


namespace logicalaccess
{
	AccessControlCardService::AccessControlCardService(boost::shared_ptr<CardProvider> cardProvider)
		: CardService(cardProvider)
	{
		
	}

	AccessControlCardService::~AccessControlCardService()
	{
	}

	CardServiceType AccessControlCardService::getServiceType() const
	{
		return CST_ACCESS_CONTROL;
	}

	boost::shared_ptr<Format> AccessControlCardService::readFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		bool ret = false;
		
		EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to read can't be null.");
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location parameter can't be null.");

		// By default duplicate the format. Other kind of implementation should override this current method.
		boost::shared_ptr<Format> formatret = Format::getByFormatType(format->getType());
		formatret->unSerialize(format->serialize(), "");

		size_t length = (formatret->getDataLength() + 7) / 8;
		if (length > 0)
		{
			unsigned char *formatBuf = new unsigned char[length];
			if (formatBuf != NULL)
			{
				memset(formatBuf, 0x00, length);
				try
				{
					if (!d_cardProvider->readData(location, aiToUse, formatBuf, length, CB_AUTOSWITCHAREA))
					{				
						THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_READ);
					}
				
					formatret->setLinearData(formatBuf, length);
					ret = true;
				}
				catch(std::exception&)
				{
					delete[] formatBuf;
					throw;
				}
				delete[] formatBuf;
			}
		}

		if (!ret)
		{
			formatret.reset();
		}

		return formatret;
	}

	bool AccessControlCardService::writeFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to write can't be null.");
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location parameter can't be null.");

		size_t length = (format->getDataLength() + 7) / 8;
		unsigned char *formatBuf = new unsigned char[length];
		memset(formatBuf, 0x00, length);
		format->getLinearData(formatBuf, length);
		try
		{
			if (!d_cardProvider->writeData(location, aiToUse, aiToWrite, formatBuf, length, CB_AUTOSWITCHAREA))
			{				
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_WRITE);
			}
		}
		catch(std::exception&)
		{
			delete[] formatBuf;
			throw;
		}

		ret = true;
		delete[] formatBuf;
		
		return ret;
	}
}

