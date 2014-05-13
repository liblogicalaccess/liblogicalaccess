/**
 * \file generictagaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GenericTag Access Control Card service.
 */

#include "generictagaccesscontrolcardservice.hpp"
#include "generictagchip.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/bithelper.hpp"


#if defined(__unix__)
#include <cstring>
#endif

namespace logicalaccess
{
	GenericTagAccessControlCardService::GenericTagAccessControlCardService(boost::shared_ptr<Chip> chip)
		: AccessControlCardService(chip)
	{
		
	}

	GenericTagAccessControlCardService::~GenericTagAccessControlCardService()
	{
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
				unsigned int dataLengthBits = static_cast<unsigned int>(getChip()->getChipIdentifier().size()) * 8;
	
				if (dataLengthBits > 0)
				{
					unsigned int length = (dataLengthBits + 7) / 8;
					unsigned formatlength = (formatret->getDataLength() + 7) / 8;
					unsigned char *formatBuf = new unsigned char[formatlength];
					memset(formatBuf, 0x00, formatlength);

					try
					{
						INFO_SIMPLE_("Reading data from Generic Tag...");
						std::vector<unsigned char> identifier = getChip()->getChipIdentifier();
						if (length <= identifier.size())
						{
							unsigned int realDataLengthBits = getGenericTagChip()->getTagIdBitsLength();
							if (realDataLengthBits == 0)
							{
								realDataLengthBits = length * 8;
							}

							if (realDataLengthBits >= formatret->getDataLength())
							{
								INFO_SIMPLE_("Converting data to format...");
								unsigned int writePosBit = 0;
								BitHelper::writeToBit(formatBuf, formatlength, &writePosBit, &identifier[0], length, dataLengthBits, dataLengthBits - realDataLengthBits, realDataLengthBits);
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
						delete[] formatBuf;
						throw;
					}
					delete[] formatBuf;
				}
			}
			catch(std::exception& ex)
			{
				ERROR_("Error on read format: %s", ex.what());
				std::rethrow_exception(std::current_exception());
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

