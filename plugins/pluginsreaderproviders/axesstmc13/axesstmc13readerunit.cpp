/**
 * \file axesstmc13readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader unit.
 */

#include "axesstmc13readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "axesstmc13readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/readerproviders/serialport.hpp"
#include "readercardadapters/axesstmc13readercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/axesstmc13datatransport.hpp"

namespace logicalaccess
{
    AxessTMC13ReaderUnit::AxessTMC13ReaderUnit()
        : ReaderUnit()
    {
        d_readerUnitConfig.reset(new AxessTMC13ReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<AxessTMC13ReaderCardAdapter>(new AxessTMC13ReaderCardAdapter()));
        std::shared_ptr<AxessTMC13DataTransport> dataTransport(new AxessTMC13DataTransport());
#ifndef UNIX
        dataTransport->setPortBaudRate(CBR_57600);
#else
        dataTransport->setPortBaudRate(B57600);
#endif
        setDataTransport(dataTransport);
        d_card_type = "UNKNOWN";

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/AxessTMC13ReaderUnit.config"), pt);
            d_card_type = pt.get("config.cardType", "UNKNOWN");
        }
        catch (...) {}
    }

    AxessTMC13ReaderUnit::~AxessTMC13ReaderUnit()
    {
        disconnectFromReader();
    }

    std::string AxessTMC13ReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string AxessTMC13ReaderUnit::getConnectedName()
    {
        return getName();
    }

    void AxessTMC13ReaderUnit::setCardType(std::string cardType)
    {
        d_card_type = cardType;
    }

    bool AxessTMC13ReaderUnit::waitInsertion(unsigned int maxwait)
    {
        bool inserted = false;
        std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);

        if (d_tmcIdentifier.size() == 0)
        {
            retrieveReaderIdentifier();
        }

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

    bool AxessTMC13ReaderUnit::waitRemoval(unsigned int maxwait)
    {
        bool removed = false;

        if (d_tmcIdentifier.size() == 0)
        {
            retrieveReaderIdentifier();
        }

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

    bool AxessTMC13ReaderUnit::connect()
    {
        return true;
    }

    void AxessTMC13ReaderUnit::disconnect()
    {
    }

    bool AxessTMC13ReaderUnit::connectToReader()
    {
        getDataTransport()->setReaderUnit(shared_from_this());
        return getDataTransport()->connect();
    }

    void AxessTMC13ReaderUnit::disconnectFromReader()
    {
        getDataTransport()->disconnect();
    }

    std::vector<unsigned char> AxessTMC13ReaderUnit::getPingCommand() const
    {
        std::vector<unsigned char> cmd;

        cmd.push_back(static_cast<unsigned char>('v'));

        return cmd;
    }

    std::shared_ptr<Chip> AxessTMC13ReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;
        std::vector<unsigned char> cmd;
        cmd.push_back(static_cast<unsigned char>('s'));

        std::vector<unsigned char> tmpASCIIId = getDefaultAxessTMC13ReaderCardAdapter()->sendCommand(cmd);
        if (tmpASCIIId.size() > 0 && tmpASCIIId[0] != 'N')
        {
            unsigned long long l = atoull(BufferHelper::getStdString(tmpASCIIId));
            char bufTmpId[64];
            sprintf(bufTmpId, "%llx", l);
            chip = ReaderUnit::createChip(
                (d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
                formatHexString(std::string(bufTmpId))
                );
        }

        return chip;
    }

    std::shared_ptr<Chip> AxessTMC13ReaderUnit::createChip(std::string type)
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

    std::shared_ptr<Chip> AxessTMC13ReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > AxessTMC13ReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<AxessTMC13ReaderCardAdapter> AxessTMC13ReaderUnit::getDefaultAxessTMC13ReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<AxessTMC13ReaderCardAdapter>(adapter);
    }

    string AxessTMC13ReaderUnit::getReaderSerialNumber()
    {
        string ret;

        return ret;
    }

    bool AxessTMC13ReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    void AxessTMC13ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void AxessTMC13ReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<AxessTMC13ReaderProvider> AxessTMC13ReaderUnit::getAxessTMC13ReaderProvider() const
    {
        return std::dynamic_pointer_cast<AxessTMC13ReaderProvider>(getReaderProvider());
    }

    bool AxessTMC13ReaderUnit::retrieveReaderIdentifier()
    {
        bool ret;
        std::vector<unsigned char> cmd;
        try
        {
            cmd.push_back(static_cast<unsigned char>('v'));

            std::vector<unsigned char> r = getDefaultAxessTMC13ReaderCardAdapter()->sendCommand(cmd);
            EXCEPTION_ASSERT_WITH_LOG(r.size() >= 3, std::invalid_argument, "Bad response getting TMC reader identifier.");
            EXCEPTION_ASSERT_WITH_LOG(r[0] == 'T', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 0 doesn't match.");
            EXCEPTION_ASSERT_WITH_LOG(r[1] == 'M', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 1 doesn't match.");
            EXCEPTION_ASSERT_WITH_LOG(r[2] == 'C', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 2 doesn't match.");

            d_tmcIdentifier = r;
            ret = true;
        }
        catch (std::invalid_argument&)
        {
            ret = false;
        }

        return ret;
    }

    std::vector<unsigned char> AxessTMC13ReaderUnit::getTMCIdentifier()
    {
        return d_tmcIdentifier;
    }
}