/**
 * \file cardsformatcomposite.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Cards format composite.
 */

#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/accessinfo.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include <boost/foreach.hpp>

namespace logicalaccess
{
	CardsFormatComposite::CardsFormatComposite()
	{
		
	}

	CardsFormatComposite::~CardsFormatComposite()
	{

	}

	boost::shared_ptr<Format> CardsFormatComposite::createFormatFromXml(const std::string& xmlstring, const std::string& rootNode)
	{
		boost::shared_ptr<Format> ret;

		std::istringstream iss(xmlstring);
		boost::property_tree::ptree pt;
		boost::property_tree::xml_parser::read_xml(iss, pt);

		boost::property_tree::ptree n;

		if (rootNode != "")
		{
			n = pt.get_child(rootNode);
		}
		else
		{
			n = pt.front().second;
		}

		if (!n.empty())
		{
			FormatType type = static_cast<FormatType>(n.get_child("<xmlattr>.type").get_value<unsigned int>());
			ret = Format::getByFormatType(type);
			ret->unSerialize(n);
		}

		return ret;
	}

	void CardsFormatComposite::addFormatForCard(std::string type, boost::shared_ptr<Format> format, boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite)
	{
		FormatInfos finfos;
		finfos.format = format;
		finfos.location = location;
		finfos.aiToUse = aiToUse;
		finfos.aiToWrite = aiToWrite;

		formatsList[type] = finfos;
	}

	void CardsFormatComposite::retrieveFormatForCard(std::string type, boost::shared_ptr<Format>* format, boost::shared_ptr<Location>* location, boost::shared_ptr<AccessInfo>* aiToUse, boost::shared_ptr<AccessInfo>* aiToWrite)
	{
		if (formatsList.find(type) != formatsList.end())
		{
			*format = formatsList[type].format;
			*location = formatsList[type].location;
			*aiToUse = formatsList[type].aiToUse;
			if (aiToWrite != NULL)
			{
				*aiToWrite = formatsList[type].aiToWrite;
			}
		}
		else
		{
			(*format).reset();
			(*location).reset();
			(*aiToUse).reset();
			if (aiToWrite != NULL)
			{
				(*aiToWrite).reset();
			}
		}
	}

	CardTypeList CardsFormatComposite::getConfiguredCardTypes()
	{
		CardTypeList ctList;
		FormatInfosList::iterator it;
		for (it = formatsList.begin(); it != formatsList.end(); ++it)
		{
			ctList.push_back(it->first);
		}
		return ctList;
	}

	void CardsFormatComposite::removeFormatForCard(std::string type)
	{
		if (formatsList.find(type) != formatsList.end())
		{
			formatsList.erase(type);
		}
	}

	boost::shared_ptr<Format> CardsFormatComposite::readFormat()
	{
		return readFormat(getReaderUnit()->getSingleChip());
	}

	boost::shared_ptr<Format> CardsFormatComposite::readFormat(boost::shared_ptr<Chip> chip)
	{
		EXCEPTION_ASSERT_WITH_LOG(getReaderUnit(), LibLogicalAccessException, "A reader unit must be associated to the object.");

		boost::shared_ptr<Format> fcopy;

		if (chip)
		{			
			INFO_("Read format using card format composite on a chip (%s)...", BufferHelper::getHex(chip->getChipIdentifier()).c_str());

			FormatInfos finfos;
			std::string ct = chip->getCardType();
			FormatInfosList::iterator it = formatsList.find(ct);
			if (it == formatsList.end())
			{
				if (chip->getCardType() != chip->getGenericCardType())
				{
					ct = chip->getGenericCardType();
					it = formatsList.find(ct);
				}
			}

			if (it != formatsList.end())
			{
				finfos = formatsList[ct];
				if (finfos.format)
				{
					// Make a manual format copy to preserve integrity.
					try
					{
						INFO_SIMPLE_("Getting access control service from chip...");
						boost::shared_ptr<AccessControlCardService> acService = boost::dynamic_pointer_cast<AccessControlCardService>(chip->getService(CST_ACCESS_CONTROL));
						if (acService)
						{
							fcopy = acService->readFormat(finfos.format, finfos.location, finfos.aiToUse);
							if (fcopy && !finfos.format->checkSkeleton(fcopy))
							{
								fcopy.reset();
							}
						}
						else
						{
							ERROR_("Cannot found any access control service for this chip.");
						}
					}
					catch(std::exception&)
					{
						fcopy.reset();
					}
				}
			}
			else
			{
				INFO_SIMPLE_("Cannot found any configured format for this chip.");
			}
		}

		if (fcopy)
		{
			INFO_SIMPLE_("Format found, return.");
		}
		else
		{
			INFO_SIMPLE_("No format found, return.");
		}

		return fcopy;
	}

	void CardsFormatComposite::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		if (formatsList.size() > 0)
		{
			FormatInfosList::iterator it;
			for (it = formatsList.begin(); it != formatsList.end(); ++it)
			{
				boost::property_tree::ptree nodecard;
				nodecard.put("type", it->first);
				nodecard.put("SelectedFormat", (it->second.format) ? it->second.format->getType() : FT_UNKNOWN);
				boost::property_tree::ptree nodeformat;
				if (it->second.format)
				{
					it->second.format->serialize(nodeformat);
					if (it->second.location)
					{
						it->second.location->serialize(nodeformat);
					}
					if (it->second.aiToUse)
					{
						it->second.aiToUse->serialize(nodeformat);
					}
					boost::property_tree::ptree nodewinfo;
					if (it->second.aiToWrite)
					{
						it->second.aiToWrite->serialize(nodewinfo);
					}
					nodecard.add_child("WriteInfo", node);
				}
				nodecard.add_child("FormatConfiguration", nodeformat);
				node.add_child("Card", nodecard);
			}
		}

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void CardsFormatComposite::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_SIMPLE_("Unserializing cards format composite...");

		EXCEPTION_ASSERT_WITH_LOG(getReaderUnit(), LibLogicalAccessException, "A reader unit must be associated to the object.");

		BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node)
		{
			if(v.first == "Card" )
			{
				string type = v.second.get_child("<xmlattr>.type").get_value<std::string>();

				boost::shared_ptr<Chip> chip = getReaderUnit()->createChip(type);
				EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException, "Unknow card type.");

				FormatInfos finfos;
				if (boost::shared_ptr<Format> format = Format::getByFormatType(static_cast<FormatType>(v.second.get_child("SelectedFormat").get_value<unsigned int>())))
				{
					boost::shared_ptr<Profile> profile = chip->getProfile();
					boost::property_tree::ptree childrenRootNode = v.second.get_child("FormatConfiguration");
					format->unSerialize(childrenRootNode, "");

					if (profile)
					{
						boost::shared_ptr<Location> location = profile->createLocation();
						if (location)
						{
							location->unSerialize(childrenRootNode);
						}
						finfos.location = location;

						boost::shared_ptr<AccessInfo> aiToUse = profile->createAccessInfo();
						if (aiToUse)
						{
							aiToUse->unSerialize(childrenRootNode);
						}
						finfos.aiToUse = aiToUse;

						boost::shared_ptr<AccessInfo> aiToWrite;
						boost::property_tree::ptree writeNode = childrenRootNode.get_child("WriteInfo");
						if (!writeNode.empty())
						{
							aiToWrite = profile->createAccessInfo();
							if (aiToWrite)
							{
								aiToWrite->unSerialize(writeNode);
							}
						}
						finfos.aiToWrite = aiToWrite;
					}
				
					finfos.format = format;							
				}
				else
				{
					finfos.format.reset();
				}

				formatsList.insert(FormatInfosPair(type, finfos));
			}
		}
	}

	std::string CardsFormatComposite::getDefaultXmlNodeName() const
	{
		return "CardsFormat";
	}

	boost::shared_ptr<ReaderUnit> CardsFormatComposite::getReaderUnit() const
	{
		return d_ReaderUnit.lock();
	}

	void CardsFormatComposite::setReaderUnit(boost::weak_ptr<ReaderUnit> unit)
	{
		d_ReaderUnit = unit;
	}
}

