/**
 * \file readerunit.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Reader unit.
 */

#include <logicalaccess/readerproviders/readerunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstring>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/readerproviders/datatransport.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <map>
#include <logicalaccess/bufferhelper.hpp>

#include <logicalaccess/readerproviders/lcddisplay.hpp>
#include <logicalaccess/readerproviders/ledbuzzerdisplay.hpp>
#include <logicalaccess/readerproviders/readerunitconfiguration.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
ReaderUnit::ReaderUnit(std::string rpt)
    : XmlSerializable()
    , d_readerProviderType(rpt)
    , d_card_type(CHIP_UNKNOWN)
{
    try
    {
        boost::property_tree::ptree pt;
        read_xml((boost::filesystem::current_path().string() + "/ReaderUnit.config"), pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

ReaderUnit::~ReaderUnit()
{
}

std::string ReaderUnit::getRPType() const
{
    return d_readerProviderType;
}

ByteVector ReaderUnit::getPingCommand() const
{
    return ByteVector();
}

std::shared_ptr<LCDDisplay> ReaderUnit::getLCDDisplay()
{
    if (d_lcdDisplay)
    {
        d_lcdDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
    }
    return d_lcdDisplay;
}

void ReaderUnit::setLCDDisplay(std::shared_ptr<LCDDisplay> d)
{
    d_lcdDisplay = d;
}

std::shared_ptr<LEDBuzzerDisplay> ReaderUnit::getLEDBuzzerDisplay()
{
    if (d_ledBuzzerDisplay)
    {
        d_ledBuzzerDisplay->setReaderCardAdapter(getDefaultReaderCardAdapter());
    }
    return d_ledBuzzerDisplay;
}

void ReaderUnit::setLEDBuzzerDisplay(std::shared_ptr<LEDBuzzerDisplay> lbd)
{
    d_ledBuzzerDisplay = lbd;
}

bool ReaderUnit::waitInsertion(const ByteVector &identifier, unsigned int maxwait)
{
    LOG(LogLevel::INFOS) << "Started for identifier " << BufferHelper::getHex(identifier)
                         << " - maxwait " << maxwait;

    bool inserted = false;

    boost::posix_time::ptime currentDate = boost::posix_time::second_clock::local_time();
    boost::posix_time::ptime maxDate =
        currentDate + boost::posix_time::milliseconds(maxwait);

    while (!inserted && currentDate < maxDate)
    {
        if (waitInsertion(maxwait))
        {
            LOG(LogLevel::INFOS)
                << "Chip(s) detected ! Looking in the list to find the chip...";
            bool found                                  = false;
            std::vector<std::shared_ptr<Chip>> chipList = getChipList();
            for (std::vector<std::shared_ptr<Chip>>::iterator i = chipList.begin();
                 i != chipList.end() && !found; ++i)
            {
                ByteVector tmp = (*i)->getChipIdentifier();
                LOG(LogLevel::INFOS) << "Processing chip " << BufferHelper::getHex(tmp)
                                     << "...";

                if (tmp == identifier)
                {
                    LOG(LogLevel::INFOS) << "Chip found !";
                    // re-assign the chip
                    d_insertedChip = *i;
                    found          = true;
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

    LOG(LogLevel::INFOS) << "Returns inserted " << inserted << " - expired "
                         << (currentDate >= maxDate);

    return inserted;
}

ByteVector ReaderUnit::getNumber(std::shared_ptr<Chip> chip,
                                 std::shared_ptr<CardsFormatComposite> composite)
{
    LOG(LogLevel::INFOS) << "Started for chip type {" << chip->getCardType()
                         << "} Generic {" << chip->getGenericCardType() << "}";
    ByteVector ret;

    if (composite)
    {
        LOG(LogLevel::INFOS) << "Composite used to find the chip identifier {"
                             << composite->serialize() << "}";
        composite->setReaderUnit(shared_from_this());

        CardTypeList ctList = composite->getConfiguredCardTypes();
        if (ctList.size() > 0)
        {
            std::string useCardType     = chip->getCardType();
            CardTypeList::iterator itct = find(ctList.begin(), ctList.end(), useCardType);
            // Try to use the generic card type
            if (itct == ctList.end())
            {
                useCardType = chip->getGenericCardType();
                LOG(LogLevel::INFOS) << "No configuration found for the chip type ! "
                                        "Looking for the generic type ("
                                     << useCardType << ") configuration...";
                itct = find(ctList.begin(), ctList.end(), useCardType);
            }
            // Try to use the configuration for all card (= generic tag), because the card
            // type isn't configured
            if (itct == ctList.end())
            {
                LOG(LogLevel::INFOS) << "No configuration found for the chip type ! "
                                        "Looking for \"GenericTag\" configuration...";
                useCardType = CHIP_GENERICTAG;
                itct        = find(ctList.begin(), ctList.end(), useCardType);
            }

            // Try to read the number only if a configuration exists (for this card type
            // or default)
            if (itct != ctList.end())
            {
                LOG(LogLevel::INFOS) << "Configuration found in the composite ! "
                                        "Retrieving format for card...";

                auto result = composite->retrieveFormatForCard(useCardType);

                if (result->getFormat())
                {
                    LOG(LogLevel::INFOS)
                        << "Format retrieved successfully ! Reading the format...";
                    result->setFormat(composite->readFormat(chip)); // Read format on chip

                    if (result->getFormat())
                    {
                        LOG(LogLevel::INFOS)
                            << "Format read successfully ! Getting identifier...";
                        ret = result->getFormat()->getIdentifier();
                    }
                    else
                    {
                        LOG(LogLevel::ERRORS) << "Unable to read the format !";
                    }
                }
                else
                {
                    LOG(LogLevel::WARNINGS) << "Cannot retrieve the format for card ! "
                                               "Trying using getNumber directly...";
                    ret = getNumber(chip);
                }
            }
            else
            {
                LOG(LogLevel::ERRORS) << "No configuration found !";
            }
        }
        else
        {
            LOG(LogLevel::INFOS) << "Composite card format is NOT NULL, but no card "
                                    "configuration ! Reader chip identifier directly...";
            ret = chip->getChipIdentifier();
        }
    }
    else
    {
        LOG(LogLevel::INFOS)
            << "Composite card format is NULL ! Reader chip identifier directly...";
        ret = chip->getChipIdentifier();
    }

    LOG(LogLevel::INFOS) << "Returns number " << BufferHelper::getHex(ret);

    return ret;
}

ByteVector ReaderUnit::getNumber(std::shared_ptr<Chip> chip)
{
    // Read encoded format if specified in the license.
    return getNumber(chip, std::shared_ptr<CardsFormatComposite>());
}

uint64_t ReaderUnit::getFormatedNumber(const ByteVector &number, int padding)
{
    LOG(LogLevel::INFOS) << "Getting formated number... number "
                         << BufferHelper::getHex(number) << " padding " << padding;
    uint64_t longnumber = 0x00;

    for (size_t i = 0, j = number.size() - 1; i < number.size(); ++i, --j)
    {
        longnumber |= ((static_cast<uint64_t>(number[i])) << (j * 8));
    }

    longnumber += padding;

    LOG(LogLevel::INFOS) << "Returns long number {0x" << std::hex << longnumber
                         << std::dec << "(" << longnumber << ")}";
    return longnumber;
}

std::string ReaderUnit::getFormatedNumber(const ByteVector &number)
{
    LOG(LogLevel::INFOS) << "Getting formated number... number "
                         << BufferHelper::getHex(number);
    std::ostringstream oss;
    oss << std::setfill('0');

    for (size_t i = 0; i < number.size(); ++i)
    {
        oss << std::hex << std::setw(2) << static_cast<unsigned int>(number[i]);
    }

    LOG(LogLevel::INFOS) << "Returns number " << oss.str();
    return oss.str();
}

std::shared_ptr<Chip> ReaderUnit::createChip(std::string type,
                                             const ByteVector &identifier)
{
    LOG(LogLevel::INFOS) << "Creating chip for card type {" << type << "} and identifier "
                         << BufferHelper::getHex(identifier) << "...";
    std::shared_ptr<Chip> chip = createChip(type);
    chip->setChipIdentifier(identifier);
    return chip;
}

std::shared_ptr<Chip> ReaderUnit::createChip(std::string type)
{
    LOG(LogLevel::INFOS) << "Attempting to create a card of type (" << type << ")";

    std::shared_ptr<Chip> ret = LibraryManager::getInstance()->getCard(type);
    if (!ret)
    {
        LOG(LogLevel::NOTICES)
            << "Falling back to creating a default Chip object because "
            << "we could'nt create a more derived object type";
        ret.reset(new Chip("__UNSUPPORTED__" + type));
    }

    return ret;
}

std::shared_ptr<ReaderCardAdapter> ReaderUnit::getDefaultReaderCardAdapter()
{
    if (d_defaultReaderCardAdapter)
    {
        if (getDataTransport())
        {
            d_defaultReaderCardAdapter->setDataTransport(getDataTransport());
        }

        if (d_defaultReaderCardAdapter->getDataTransport())
        {
            d_defaultReaderCardAdapter->getDataTransport()->setReaderUnit(
                shared_from_this());
        }
    }
    return d_defaultReaderCardAdapter;
}

void ReaderUnit::setDefaultReaderCardAdapter(
    std::shared_ptr<ReaderCardAdapter> defaultRca)
{
    d_defaultReaderCardAdapter = defaultRca;
}

std::shared_ptr<DataTransport> ReaderUnit::getDataTransport() const
{
    return d_dataTransport;
}

void ReaderUnit::setDataTransport(std::shared_ptr<DataTransport> dataTransport)
{
    d_dataTransport = dataTransport;
}

std::shared_ptr<ReaderUnitConfiguration> ReaderUnit::getConfiguration() const
{
    // LOG(LogLevel::INFOS) << "Getting reader unit configuration...");
    if (d_readerUnitConfig)
    {
        // LOG(LogLevel::INFOS) << "Reader unit configuration {%s}",
        // d_readerUnitConfig->serialize().c_str());
    }

    return d_readerUnitConfig;
}

void ReaderUnit::setConfiguration(std::shared_ptr<ReaderUnitConfiguration> config)
{
    d_readerUnitConfig = config;
}

std::string ReaderUnit::getDefaultXmlNodeName() const
{
    return "ReaderUnit";
}

void ReaderUnit::serialize(boost::property_tree::ptree &node)
{
    node.put("<xmlattr>.type", getReaderProvider()->getRPType());

    if (d_readerUnitConfig)
        d_readerUnitConfig->serialize(node);
    if (getDataTransport())
    {
        node.put("TransportType", getDataTransport()->getTransportType());
        getDataTransport()->serialize(node);
    }
    else
    {
        node.put("TransportType", "");
    }
}

void ReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    disconnectFromReader();
    if (d_readerUnitConfig)
        d_readerUnitConfig->unSerialize(
            node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
    std::string transportType = node.get_child("TransportType").get_value<std::string>();
    std::shared_ptr<DataTransport> dataTransport =
        LibraryManager::getInstance()->getDataTransport(transportType);
    // Cannot create data transport instance from xml, use default one
    if (!dataTransport)
    {
        dataTransport = getDataTransport();
    }
    if (dataTransport && transportType == dataTransport->getTransportType())
    {
        dataTransport->unSerialize(
            node.get_child(dataTransport->getDefaultXmlNodeName()));
        setDataTransport(dataTransport);
    }
}

void ReaderUnit::unSerialize(boost::property_tree::ptree &node,
                             const std::string &rootNode)
{
    boost::property_tree::ptree parentNode = node.get_child(rootNode);
    BOOST_FOREACH (boost::property_tree::ptree::value_type const &v, parentNode)
    {
        if (v.first == getDefaultXmlNodeName())
        {
            if (static_cast<std::string>(
                    v.second.get_child("<xmlattr>.type").get_value<std::string>()) ==
                getReaderProvider()->getRPType())
            {
                boost::property_tree::ptree r = v.second;
                unSerialize(r);
            }
        }
    }
}

void ReaderUnit::setCardTechnologies(const TechnoBitset & /*bitset*/)
{
}

TechnoBitset ReaderUnit::getCardTechnologies()
{
    TechnoBitset tb = 0;
    return tb;
}


TechnoBitset ReaderUnit::getPossibleCardTechnologies()
{
    TechnoBitset tb = 0;
    return tb;
}

std::shared_ptr<ResultChecker> ReaderUnit::createDefaultResultChecker() const
{
    return nullptr;
}

std::shared_ptr<CardProbe> ReaderUnit::createCardProbe()
{
    return nullptr;
}

ReaderServicePtr ReaderUnit::getService(const ReaderServiceType & /*type*/)
{
    return nullptr;
}
}
