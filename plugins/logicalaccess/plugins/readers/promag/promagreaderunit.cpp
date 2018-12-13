/**
 * \file promagreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag reader unit.
 */

#include <logicalaccess/plugins/readers/promag/promagreaderunit.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/readers/promag/promagreaderprovider.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/readers/promag/readercardadapters/promagreadercardadapter.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/plugins/readers/promag/readercardadapters/promagserialportdatatransport.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
PromagReaderUnit::PromagReaderUnit()
    : ReaderUnit(READER_PROMAG)
{
    d_readerUnitConfig.reset(new PromagReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<PromagReaderCardAdapter>());
    std::shared_ptr<PromagSerialPortDataTransport> dataTransport(new PromagSerialPortDataTransport());
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = CHIP_UNKNOWN;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/PromagReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
    }
    catch (...)
    {
    }
}

PromagReaderUnit::~PromagReaderUnit()
{
    PromagReaderUnit::disconnectFromReader();
}

std::string PromagReaderUnit::getName() const
{
    return getDataTransport()->getName();
}

std::string PromagReaderUnit::getConnectedName()
{
    return getName();
}

void PromagReaderUnit::setCardType(std::string cardType)
{
    d_card_type = cardType;
}

bool PromagReaderUnit::waitInsertion(unsigned int maxwait)
{
    bool inserted = false;
    std::chrono::steady_clock::time_point const clock_timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);

    if (d_promagIdentifier.size() == 0)
    {
        retrieveReaderIdentifier();
    }

    do
    {
        std::shared_ptr<Chip> chip = getChipInAir();
        if (chip)
        {
            d_insertedChip = chip;
            inserted       = true;
        }

        if (!inserted)
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
    } while (!inserted && std::chrono::steady_clock::now() < clock_timeout);

    return inserted;
}

bool PromagReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool removed = false;

    if (d_promagIdentifier.size() == 0)
    {
        retrieveReaderIdentifier();
    }

    if (d_insertedChip)
    {
        std::chrono::steady_clock::time_point const clock_timeout =
            std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);
        do
        {
            std::shared_ptr<Chip> chip = getChipInAir();
            if (chip)
            {
                if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
                {
                    d_insertedChip.reset();
                    removed = true;
                }
            }
            else
            {
                d_insertedChip.reset();
                removed = true;
            }

            if (!removed)
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
        } while (!removed && std::chrono::steady_clock::now() < clock_timeout);
    }

    return removed;
}

bool PromagReaderUnit::connect()
{
    return true;
}

void PromagReaderUnit::disconnect()
{
}

ByteVector PromagReaderUnit::getPingCommand() const
{
    ByteVector cmd;

    cmd.push_back(static_cast<unsigned char>('N'));

    return cmd;
}

std::shared_ptr<Chip> PromagReaderUnit::getChipInAir()
{
    std::shared_ptr<Chip> chip;
    ByteVector cmd;
    cmd.push_back(static_cast<unsigned char>('R'));

    ByteVector rawidbuf = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);
    if (rawidbuf.size() > 0)
    {
        std::string rawidstr  = BufferHelper::getStdString(rawidbuf);
        ByteVector insertedId = formatHexString(rawidstr);
        chip                  = ReaderUnit::createChip(
            (d_card_type == CHIP_UNKNOWN ? CHIP_GENERICTAG : d_card_type), insertedId);
    }

    return chip;
}

std::shared_ptr<Chip> PromagReaderUnit::createChip(std::string type)
{
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    if (chip)
    {
        std::shared_ptr<ReaderCardAdapter> rca;

        if (type == CHIP_GENERICTAG)
            rca = getDefaultReaderCardAdapter();
        else
            return chip;

        rca->setDataTransport(getDataTransport());
    }
    return chip;
}

std::shared_ptr<Chip> PromagReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> PromagReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::shared_ptr<PromagReaderCardAdapter>
PromagReaderUnit::getDefaultPromagReaderCardAdapter()
{
    std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
    return std::dynamic_pointer_cast<PromagReaderCardAdapter>(adapter);
}

std::string PromagReaderUnit::getReaderSerialNumber()
{
    return std::string();
}

bool PromagReaderUnit::isConnected()
{
    return bool(d_insertedChip);
}

bool PromagReaderUnit::connectToReader()
{
    return getDataTransport()->connect();
}

void PromagReaderUnit::disconnectFromReader()
{
    getDataTransport()->disconnect();
}

void PromagReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void PromagReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<PromagReaderProvider> PromagReaderUnit::getPromagReaderProvider() const
{
    return std::dynamic_pointer_cast<PromagReaderProvider>(getReaderProvider());
}

bool PromagReaderUnit::retrieveReaderIdentifier()
{
    bool ret;
    ByteVector cmd;
    try
    {
        cmd.push_back(static_cast<unsigned char>('N'));

        ByteVector r = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);

        d_promagIdentifier = r;
        ret                = true;
    }
    catch (std::exception &)
    {
        ret = false;
    }

    return ret;
}

ByteVector PromagReaderUnit::getPromagIdentifier() const
{
    return d_promagIdentifier;
}
}