/**
 * \file elatecreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec reader unit.
 */

#include "elatecreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "elatecreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/elatecreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/elatecdatatransport.hpp"
#include "readercardadapters/elatecbufferparser.hpp"

namespace logicalaccess
{
    ElatecReaderUnit::ElatecReaderUnit()
        : ReaderUnit()
    {
        d_readerUnitConfig.reset(new ElatecReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<ElatecReaderCardAdapter>(new ElatecReaderCardAdapter()));
        std::shared_ptr<ElatecDataTransport> dataTransport(new ElatecDataTransport());
        setDataTransport(dataTransport);
        d_card_type = "UNKNOWN";

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/ElatecReaderUnit.config"), pt);
            d_card_type = pt.get("config.cardType", "UNKNOWN");
        }
        catch (...) {}
    }

    ElatecReaderUnit::~ElatecReaderUnit()
    {
        disconnectFromReader();
    }

    std::string ElatecReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string ElatecReaderUnit::getConnectedName()
    {
        return getName();
    }

    void ElatecReaderUnit::setCardType(std::string cardType)
    {
        d_card_type = cardType;
    }

    bool ElatecReaderUnit::waitInsertion(unsigned int maxwait)
    {
        bool inserted = false;
        std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);

        do
        {
            std::shared_ptr<Chip> chip = getChipInAir();
            if (chip)
            {
                d_insertedChip = chip;
                inserted = true;
            }

            if (!inserted)
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
        } while (!inserted && std::chrono::steady_clock::now() < clock_timeout);

        return inserted;
    }

    bool ElatecReaderUnit::waitRemoval(unsigned int maxwait)
    {
        bool removed = false;

        if (d_insertedChip)
        {
            std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);
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

    bool ElatecReaderUnit::connect()
    {
        return bool(d_insertedChip);
    }

    void ElatecReaderUnit::disconnect()
    {
    }

    std::shared_ptr<Chip> ElatecReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;

        std::vector<unsigned char> result = getDefaultElatecReaderCardAdapter()->sendCommand(0x11, std::vector<unsigned char>());
        if (result.size() > 0)
        {
            chip = ReaderUnit::createChip(
                (d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
                std::vector<unsigned char>(result.begin() + 1, result.end())
                );
        }

        return chip;
    }

    std::shared_ptr<Chip> ElatecReaderUnit::createChip(std::string type)
    {
        std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

        if (chip)
        {
            std::shared_ptr<ReaderCardAdapter> rca;

            if (type == "GenericTag")
                rca = getDefaultReaderCardAdapter();
            else
                return chip;

            rca->setDataTransport(getDataTransport());
        }
        return chip;
    }

    std::shared_ptr<Chip> ElatecReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > ElatecReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<ElatecReaderCardAdapter> ElatecReaderUnit::getDefaultElatecReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<ElatecReaderCardAdapter>(adapter);
    }

    string ElatecReaderUnit::getReaderSerialNumber()
    {
        string ret;

        return ret;
    }

    bool ElatecReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    bool ElatecReaderUnit::connectToReader()
    {
        getDataTransport()->setReaderUnit(shared_from_this());
        return getDataTransport()->connect();
    }

    void ElatecReaderUnit::disconnectFromReader()
    {
        getDataTransport()->disconnect();
    }

    std::vector<unsigned char> ElatecReaderUnit::getPingCommand() const
    {
        std::vector<unsigned char> cmd;

        cmd.push_back(0x11);

        return cmd;
    }

    void ElatecReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void ElatecReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<ElatecReaderProvider> ElatecReaderUnit::getElatecReaderProvider() const
    {
        return std::dynamic_pointer_cast<ElatecReaderProvider>(getReaderProvider());
    }
}