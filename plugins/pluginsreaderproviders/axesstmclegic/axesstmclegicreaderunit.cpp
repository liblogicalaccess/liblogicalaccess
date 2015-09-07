/**
 * \file axesstmclegicreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic reader unit.
 */

#include "axesstmclegicreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "axesstmclegicreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/axesstmclegicreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/axesstmclegicdatatransport.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    AxessTMCLegicReaderUnit::AxessTMCLegicReaderUnit()
        : ReaderUnit()
    {
        d_readerUnitConfig.reset(new AxessTMCLegicReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<AxessTMCLegicReaderCardAdapter>(new AxessTMCLegicReaderCardAdapter()));
        std::shared_ptr<AxessTMCLegicDataTransport> dataTransport(new AxessTMCLegicDataTransport());
        dataTransport->setPortBaudRate(57600);
        setDataTransport(dataTransport);
        d_card_type = "UNKNOWN";

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/AxessTMCLegicReaderUnit.config"), pt);
            d_card_type = pt.get("config.cardType", "UNKNOWN");
        }
        catch (...) {}
    }

    AxessTMCLegicReaderUnit::~AxessTMCLegicReaderUnit()
    {
        disconnectFromReader();
    }

    std::string AxessTMCLegicReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string AxessTMCLegicReaderUnit::getConnectedName()
    {
        return getName();
    }

    void AxessTMCLegicReaderUnit::setCardType(std::string cardType)
    {
        d_card_type = cardType;
    }

    bool AxessTMCLegicReaderUnit::waitInsertion(unsigned int maxwait)
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

    bool AxessTMCLegicReaderUnit::waitRemoval(unsigned int maxwait)
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

    bool AxessTMCLegicReaderUnit::connect()
    {
        return true;
    }

    void AxessTMCLegicReaderUnit::disconnect()
    {
    }

    bool AxessTMCLegicReaderUnit::connectToReader()
    {
        getDataTransport()->setReaderUnit(shared_from_this());
        return getDataTransport()->connect();
    }

    void AxessTMCLegicReaderUnit::disconnectFromReader()
    {
        getDataTransport()->disconnect();
    }

    std::vector<unsigned char> AxessTMCLegicReaderUnit::getPingCommand() const
    {
        std::vector<unsigned char> cmd;

        cmd.push_back(0xb0);
        cmd.push_back(0xb2);

        return cmd;
    }

    void AxessTMCLegicReaderUnit::idleState()
    {
        std::vector<unsigned char> cmd;
        cmd.push_back(0x14);
        cmd.push_back(0x01);
        cmd.push_back(0x16);

        getDefaultAxessTMCLegicReaderCardAdapter()->sendCommand(cmd);
    }

    std::shared_ptr<Chip> AxessTMCLegicReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;

#ifdef _WINDOWS
        std::vector<unsigned char> cmd;
        cmd.push_back(0xb0);
        cmd.push_back(0xb2);

        std::vector<unsigned char> response = getDefaultAxessTMCLegicReaderCardAdapter()->sendCommand(cmd);
        if (response.size() > 4)
        {
            unsigned char idlength = response[4];
            EXCEPTION_ASSERT_WITH_LOG((response.size() - 5) >= idlength, LibLogicalAccessException, "Bad id buffer length.");

            chip = ReaderUnit::createChip(
                (d_card_type == "UNKNOWN" ? "LegicPrime" : d_card_type),
                std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + idlength)
                );
        }
#endif

        return chip;
    }

    std::shared_ptr<Chip> AxessTMCLegicReaderUnit::createChip(std::string type)
    {
        std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

        if (chip)
        {
            std::shared_ptr<ReaderCardAdapter> rca;

            if (type == "LegicPrime")
                rca = getDefaultReaderCardAdapter();
            else
                return chip;

            rca->setDataTransport(getDataTransport());
        }
        return chip;
    }

    std::shared_ptr<Chip> AxessTMCLegicReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > AxessTMCLegicReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<AxessTMCLegicReaderCardAdapter> AxessTMCLegicReaderUnit::getDefaultAxessTMCLegicReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<AxessTMCLegicReaderCardAdapter>(adapter);
    }

    std::string AxessTMCLegicReaderUnit::getReaderSerialNumber()
    {
        std::string ret;

        return ret;
    }

    bool AxessTMCLegicReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    void AxessTMCLegicReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void AxessTMCLegicReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<AxessTMCLegicReaderProvider> AxessTMCLegicReaderUnit::getAxessTMCLegicReaderProvider() const
    {
        return std::dynamic_pointer_cast<AxessTMCLegicReaderProvider>(getReaderProvider());
    }
}