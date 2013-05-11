/**
 * \file ReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader unit.
 */

#include "logicalaccess/ReaderProviders/ReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/Cards/ReaderCardAdapter.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Services/AccessControl/Formats/StaticFormat.h"
#include "logicalaccess/Cards/Chip.h"
#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include "logicalaccess/DynLibrary/LibraryManager.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>


namespace logicalaccess
{
	ReaderUnit::ReaderUnit()
		: XmlSerializable()
	{
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	ReaderUnit::~ReaderUnit()
	{
	}

	boost::shared_ptr<LCDDisplay> ReaderUnit::getLCDDisplay()
	{
		d_lcdDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
		return d_lcdDisplay;
	}

	boost::shared_ptr<LEDBuzzerDisplay> ReaderUnit::getLEDBuzzerDisplay()
	{
		d_ledBuzzerDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
		return d_ledBuzzerDisplay;
	}

	bool ReaderUnit::waitInsertion(const std::vector<unsigned char>& identifier, unsigned int maxwait)
	{
		INFO_("Started for identifier %s - maxwait {%u}", BufferHelper::getHex(identifier).c_str(), maxwait);

		bool inserted = false;

		boost::posix_time::ptime currentDate = boost::posix_time::second_clock::local_time();
		boost::posix_time::ptime maxDate = currentDate + boost::posix_time::milliseconds(maxwait);

		while (!inserted && currentDate < maxDate)
		{
			if (waitInsertion(maxwait))
			{
				INFO_SIMPLE_("Chip(s) detected ! Looking in the list to find the chip...");
				bool found = false;
				std::vector<boost::shared_ptr<Chip> > chipList = getChipList();
				for (std::vector<boost::shared_ptr<Chip> >::iterator i = chipList.begin(); i != chipList.end() && !found; ++i)
				{
					std::vector<unsigned char> tmp = (*i)->getChipIdentifier();
					INFO_("Processing chip %s...", BufferHelper::getHex(tmp).c_str());

					if (tmp == identifier)
					{
						INFO_SIMPLE_("Chip found !");
						// re-assign the chip
						d_insertedChip = *i;
						found = true;
					}
				}

				if (found)
				{
					inserted = true;
				}
				else
				{
					d_insertedChip.reset();
				}
			}
			currentDate = boost::posix_time::second_clock::local_time();
		}

		INFO_("Returns inserted {%d} - expired {%d}", inserted, (currentDate >= maxDate));

		return inserted;
	}

	std::vector<unsigned char> ReaderUnit::getNumber(boost::shared_ptr<Chip> chip, boost::shared_ptr<CardsFormatComposite> composite)
	{
		INFO_("Started for chip type {0x%s(%s)}", chip->getGenericCardType().c_str(), chip->getGenericCardType().c_str());
		std::vector<unsigned char> ret;

		if (composite)
		{
			INFO_("Composite used to find the chip identifier {%s}", dynamic_cast<XmlSerializable*>(&(*composite))->serialize().c_str());
			composite->setReaderUnit(shared_from_this());

			CardTypeList ctList = composite->getConfiguredCardTypes();
			std::string useCardType = chip->getGenericCardType();
			CardTypeList::iterator itct = std::find(ctList.begin(), ctList.end(), useCardType);
			// Try to use the configuration for all card (= generic tag), because the card type isn't configured
			if (itct == ctList.end())
			{
				WARNING_SIMPLE_("No configuration found for the chip type ! Looking for \"GenericTag\" configuration...");
				useCardType = "GenericTag";
				itct = std::find(ctList.begin(), ctList.end(), useCardType);
			}

			// Try to read the number only if a configuration exists (for this card type or default)
			if (itct != ctList.end())
			{
				INFO_SIMPLE_("Configuration found in the composite ! Retrieving format for card...");
				boost::shared_ptr<LOGICALACCESS::AccessInfo> ai;
				boost::shared_ptr<LOGICALACCESS::Location> loc;
				boost::shared_ptr<LOGICALACCESS::Format> format;
				composite->retrieveFormatForCard(useCardType, &format, &loc, &ai);

				if (format)
				{
					INFO_SIMPLE_("Format retrieved successfully ! Reading the format...");
					format = composite->readFormat();	// Read format on chip

					if (format)
					{
						INFO_SIMPLE_("Format read successfully ! Getting identifier...");
						ret = format->getIdentifier();
					}
					else
					{
						ERROR_SIMPLE_("Unable to read the format !");
					}
				}
				else
				{
					WARNING_SIMPLE_("Cannot retrieve the format for card ! Trying using getNumber directly...");
					ret = getNumber(chip);
				}
			}
			else
			{
				ERROR_SIMPLE_("No configuration found !");
			}
		}
		else
		{
			INFO_SIMPLE_("Composite card format is NULL ! Reader chip identifier directly...");
			ret = chip->getChipIdentifier();
		}

		INFO_("Returns number %s", BufferHelper::getHex(ret).c_str());

		return ret;
	}

	std::vector<unsigned char> ReaderUnit::getNumber(boost::shared_ptr<Chip> chip)
	{
		INFO_("Started for chip type {0x%s(%s)}.", chip->getGenericCardType().c_str(), chip->getGenericCardType().c_str());

#ifdef _LICENSE_SYSTEM
		INFO_SIMPLE_("Licence system activated ! Getting card format composite from license...");
#endif

		// Read encoded format if specified in the license.
		return getNumber(chip,
#ifdef _LICENSE_SYSTEM
			d_license->getCardsFormatComposite(shared_from_this())
#else
			boost::shared_ptr<CardsFormatComposite>()
#endif
		);		
	}

	uint64_t ReaderUnit::getFormatedNumber(const std::vector<unsigned char>& number, int padding)
	{
		INFO_("Getting formated number... number %s-{%s} padding {%d}", BufferHelper::getHex(number).c_str(), padding);
		uint64_t longnumber = 0x00;

		for (size_t i = 0, j = number.size()-1; i < number.size(); ++i, --j)
		{
			longnumber |= ((static_cast<uint64_t>(number[i])) << (j*8));
		}

		longnumber += padding;

		INFO_("Returns long number {0x%lx(%ld)}", longnumber, longnumber);
		return longnumber;
	}

	string ReaderUnit::getFormatedNumber(const std::vector<unsigned char>& number)
	{
		INFO_("Getting formated number... number %s", BufferHelper::getHex(number).c_str());
		std::ostringstream oss;
		oss << std::setfill('0');

		for (size_t i = 0; i < number.size(); ++i)
		{
			oss << std::hex << std::setw(2) << static_cast<unsigned int>(number[i]);
		}

		INFO_("Returns number {%s}", oss.str().c_str()); 
		return oss.str();
	}

	boost::shared_ptr<Chip> ReaderUnit::createChip(std::string type, const std::vector<unsigned char>& identifier)
	{
		INFO_("Creating chip for card type {0x%s(%s)} and identifier %s...", type.c_str(), type.c_str(), BufferHelper::getHex(identifier).c_str());
		boost::shared_ptr<Chip> chip = createChip(type);
		chip->setChipIdentifier(identifier);
		return chip;
	}

	boost::shared_ptr<Chip> ReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> ret = LibraryManager::getInstance()->getCard(type);

		if (!ret)
		{
			ret.reset(new Chip(type));
		}

		return ret;
	}

	boost::shared_ptr<ReaderCardAdapter> ReaderUnit::getDefaultReaderCardAdapter()
	{
		d_defaultReaderCardAdapter->setReaderUnit(shared_from_this());
		return d_defaultReaderCardAdapter;
	}

	boost::shared_ptr<ReaderUnitConfiguration> ReaderUnit::getConfiguration()
	{
		//INFO_SIMPLE_("Getting reader unit configuration...");
		if (d_readerUnitConfig)
		{
			//INFO_("Reader unit configuration {%s}", d_readerUnitConfig->serialize().c_str());
		}

		return d_readerUnitConfig;
	}

	void ReaderUnit::setConfiguration(boost::shared_ptr<ReaderUnitConfiguration> config)
	{
		INFO_SIMPLE_("Setting reader unit configuration...");
		d_readerUnitConfig = config;
		if (d_readerUnitConfig)
		{
			INFO_("Reader unit configuration {%s}", d_readerUnitConfig->serialize().c_str());
		}
	}

	std::string ReaderUnit::getDefaultXmlNodeName() const
	{
		return "ReaderUnit";
	}

	bool ReaderUnit::unSerialize(boost::property_tree::ptree& node, const std::string& rootNode)
	{
		try
		{
			boost::property_tree::ptree parentNode = node.get_child(rootNode);
			BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, parentNode)
			{
				if (v.first == getDefaultXmlNodeName())
				{
					if (static_cast<std::string>(v.second.get_child("<xmlattr>.type").get_value<std::string>()) == getReaderProvider()->getRPType())
					{
						boost::property_tree::ptree r = v.second;
						dynamic_cast<XmlSerializable*>(this)->unSerialize(r);
						
						return true;
					}
				}
			}
		}
		catch (std::exception& e)
		{
			ERROR_("Exception {%s} !", e.what());
		}
		catch (...)
		{
			ERROR_SIMPLE_("Exception occured !");
		}

		return false;
	}
}
