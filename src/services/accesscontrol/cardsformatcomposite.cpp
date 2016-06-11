/**
 * \file cardsformatcomposite.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Cards format composite.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/accessinfo.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
    CardsFormatComposite::CardsFormatComposite()
    {
    }

    CardsFormatComposite::~CardsFormatComposite()
    {
    }

    std::shared_ptr<Format> CardsFormatComposite::createFormatFromXml(const std::string& xmlstring, const std::string& rootNode)
    {
        std::shared_ptr<Format> ret;

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

    void CardsFormatComposite::addFormatForCard(std::string type, std::shared_ptr<Format> format, std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse, std::shared_ptr<AccessInfo> aiToWrite)
    {
        LOG(LogLevel::INFOS) << "Add format for card type {" << type << "}...";
        FormatInfos finfos;
        finfos.format = format;
        finfos.location = location;
        finfos.aiToUse = aiToUse;
        finfos.aiToWrite = aiToWrite;

        formatsList[type] = finfos;
    }

    void CardsFormatComposite::retrieveFormatForCard(std::string type, std::shared_ptr<Format>* format, std::shared_ptr<Location>* location, std::shared_ptr<AccessInfo>* aiToUse, std::shared_ptr<AccessInfo>* aiToWrite)
    {
        LOG(LogLevel::INFOS) << "Retrieving format for card type {" << type << "}...";
        if (formatsList.find(type) != formatsList.end())
        {
            LOG(LogLevel::INFOS) << "Type found int the composite. Retrieving values...";
            *format = formatsList[type].format;
            *location = formatsList[type].location;
            *aiToUse = formatsList[type].aiToUse;
            if (aiToWrite != NULL)
            {
                *aiToWrite = formatsList[type].aiToWrite;
            }
            else
            {
                LOG(LogLevel::ERRORS) << "aiToWrite parameter is null. Cannot retrieve write value.";
            }
        }
        else
        {
            LOG(LogLevel::INFOS) << "No format found for this type.";
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
        formatsList.erase(type);
    }

    std::shared_ptr<Format> CardsFormatComposite::readFormat()
    {
        return readFormat(getReaderUnit()->getSingleChip());
    }

    std::shared_ptr<Format> CardsFormatComposite::readFormat(std::shared_ptr<Chip> chip)
    {
        EXCEPTION_ASSERT_WITH_LOG(getReaderUnit(), LibLogicalAccessException, "A reader unit must be associated to the object.");

        std::shared_ptr<Format> fcopy;

        if (chip)
        {
            LOG(LogLevel::INFOS) << "Read format using card format composite on a chip (" << BufferHelper::getHex(chip->getChipIdentifier()) << ")...";

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
                        LOG(LogLevel::INFOS) << "Getting access control service from chip...";
                        std::shared_ptr<AccessControlCardService> acService = std::dynamic_pointer_cast<AccessControlCardService>(chip->getService(CST_ACCESS_CONTROL));
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
                            LOG(LogLevel::ERRORS) << "Cannot found any access control service for this chip.";
                        }
                    }
                    catch (std::exception& ex)
                    {
                        LOG(LogLevel::WARNINGS) << "Read format failed: {" << ex.what() << "}";
                        fcopy.reset();
                    }
                }
            }
            else
            {
                LOG(LogLevel::INFOS) << "Cannot found any configured format for this chip.";
            }
        }

        if (fcopy)
        {
            LOG(LogLevel::INFOS) << "Format found, return.";
        }
        else
        {
            LOG(LogLevel::INFOS) << "No format found, return.";
        }

        return fcopy;
    }

    void CardsFormatComposite::serialize(boost::property_tree::ptree& parentNode)
    {
        LOG(LogLevel::INFOS) << "Serializing card format composite...";
        boost::property_tree::ptree node;

        if (!formatsList.empty())
        {
            FormatInfosList::iterator it;
            for (it = formatsList.begin(); it != formatsList.end(); ++it)
            {
                LOG(LogLevel::INFOS) << "Serializing type {" << it->first << "}...";
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
                        LOG(LogLevel::INFOS) << "Write info detected. Serializing...";
                        it->second.aiToWrite->serialize(nodewinfo);
                    }
                    nodecard.add_child("WriteInfo", nodewinfo);
                }
                nodecard.add_child("FormatConfiguration", nodeformat);
                node.add_child("Card", nodecard);
            }
        }

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void CardsFormatComposite::unSerialize(boost::property_tree::ptree& node)
    {
        LOG(LogLevel::INFOS) << "Unserializing cards format composite...";

        EXCEPTION_ASSERT_WITH_LOG(getReaderUnit(), LibLogicalAccessException, "A reader unit must be associated to the object.");

        BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node)
        {
            if (v.first == "Card")
            {
                string type = v.second.get_child("type").get_value<std::string>();

                std::shared_ptr<Chip> chip = getReaderUnit()->createChip(type);
                EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException, "Unknow card type.");

                FormatInfos finfos;
                if (std::shared_ptr<Format> format = Format::getByFormatType(static_cast<FormatType>(v.second.get_child("SelectedFormat").get_value<unsigned int>())))
                {
                    boost::property_tree::ptree childrenRootNode = v.second.get_child("FormatConfiguration");
                    format->unSerialize(childrenRootNode, "");

                    std::shared_ptr<Location> location = chip->createLocation();
                    if (location)
                    {
                        try
                        {
                            location->unSerialize(childrenRootNode, "");
                        }
                        catch (std::exception& ex)
                        {
                            LOG(LogLevel::ERRORS) << "Cannot unserialize location: {" << ex.what() << "}";
                            location.reset();
                        }
                    }
                    finfos.location = location;

					std::shared_ptr<AccessInfo> aiToUse = chip->createAccessInfo();
                    if (aiToUse)
                    {
                        try
                        {
                            aiToUse->unSerialize(childrenRootNode, "");
                        }
                        catch (std::exception& ex)
                        {
                            LOG(LogLevel::ERRORS) << "Cannot unserialize access info to use: {" << ex.what() << "}";
                            aiToUse.reset();
                        }
                    }
                    finfos.aiToUse = aiToUse;

                    std::shared_ptr<AccessInfo> aiToWrite;
                    boost::property_tree::ptree writeNode = v.second.get_child("WriteInfo");
                    if (!writeNode.empty())
                    {
						aiToWrite = chip->createAccessInfo();
                        if (aiToWrite)
                        {
                            try
                            {
                                aiToWrite->unSerialize(writeNode, "");
                            }
                            catch (std::exception& ex)
                            {
                                LOG(LogLevel::ERRORS) << "Cannot unserialize access info to write: {" << ex.what() << "}";
                                aiToWrite.reset();
                            }
                        }
                    }
                    finfos.aiToWrite = aiToWrite;

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

    std::shared_ptr<ReaderUnit> CardsFormatComposite::getReaderUnit() const
    {
        return d_ReaderUnit.lock();
    }

    void CardsFormatComposite::setReaderUnit(std::weak_ptr<ReaderUnit> unit)
    {
        d_ReaderUnit = unit;
    }
}