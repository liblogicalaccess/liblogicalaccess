/**
 * \file idondemandreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand reader unit.
 */

#include "idondemandreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "idondemandreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/idondemandreadercardadapter.hpp"
#include "generictagidondemandchip.hpp"
#include "commands/generictagidondemandaccesscontrolcardservice.hpp"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "commands/generictagidondemandcommands.hpp"
#include "logicalaccess/myexception.hpp"
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
    IdOnDemandReaderUnit::IdOnDemandReaderUnit()
        : ReaderUnit(READER_IDONDEMAND)
    {
        d_readerUnitConfig.reset(new IdOnDemandReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<IdOnDemandReaderCardAdapter>(new IdOnDemandReaderCardAdapter()));
        std::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
        setDataTransport(dataTransport);
		d_card_type = CHIP_UNKNOWN;

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/IdOnDemandReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
        }
        catch (...) {}
    }

    IdOnDemandReaderUnit::~IdOnDemandReaderUnit()
    {
        disconnectFromReader();
    }

    std::string IdOnDemandReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string IdOnDemandReaderUnit::getConnectedName()
    {
        return getName();
    }

    void IdOnDemandReaderUnit::setCardType(std::string cardType)
    {
        LOG(LogLevel::INFOS) << "Setting card type {" << cardType << "}";
        d_card_type = cardType;
    }

    void IdOnDemandReaderUnit::authenticateSDK()
    {
        authenticateSDK(getIdOnDemandConfiguration()->getAuthCode());
    }

    void IdOnDemandReaderUnit::authenticateSDK(std::string authCode)
    {
        char cmd[64];
#if defined(UNIX)
        sprintf(cmd, "AUTH %s", authCode.c_str());
#else
        sprintf_s(cmd, sizeof(cmd), "AUTH %s", authCode.c_str());
#endif
        std::string strcmd = std::string(cmd);
        getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
    }

    void IdOnDemandReaderUnit::beep()
    {
        std::string strcmd = std::string("BEEP");
        getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
    }

    bool IdOnDemandReaderUnit::read()
    {
        bool ret = true;
        try
        {
            std::string strcmd = std::string("READ");
            getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
        }
        catch (CardException& e)
        {
            LOG(LogLevel::ERRORS) << "Exception Read {" << e.what() << "}";
            ret = false;
        }

        return ret;
    }

    bool IdOnDemandReaderUnit::verify()
    {
        bool ret = true;
        try
        {
            std::string strcmd = std::string("VERIFY");
            getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
        }
        catch (CardException& e)
        {
            LOG(LogLevel::ERRORS) << "Exception Verify {" << e.what() << "}";
            ret = false;
        }

        return ret;
    }

    bool IdOnDemandReaderUnit::write()
    {
        bool ret = true;
        try
        {
            std::string strcmd = std::string("WRITE");
            getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()), 4000);
        }
        catch (CardException& e)
        {
            LOG(LogLevel::ERRORS) << "Exception Write {" << e.what() << "}";
            ret = false;
        }

        return ret;
    }

    void IdOnDemandReaderUnit::activateEMCard(bool activate)
    {
        std::string strcmd = "EMCARD ";
        if (activate)
        {
            strcmd += "ON";
        }
        else
        {
            strcmd += "FALSE";
        }
        getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
    }

    void IdOnDemandReaderUnit::activateAtmelCard(bool activate)
    {
        std::string strcmd = "ATCARD ";
        if (activate)
        {
            strcmd += "ON";
        }
        else
        {
            strcmd += "FALSE";
        }
        getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
    }

    bool IdOnDemandReaderUnit::waitInsertion(unsigned int maxwait)
    {
        bool inserted = false;
        unsigned int currentWait = 0;

        do
        {
            std::shared_ptr<Chip> chip = getChipInAir();
            if (chip)
            {
                d_insertedChip = chip;
                inserted = true;
            }

            if (!inserted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                currentWait += 100;
            }
        } while (!inserted && (maxwait == 0 || currentWait < maxwait));

        return inserted;
    }

    bool IdOnDemandReaderUnit::waitRemoval(unsigned int maxwait)
    {
        bool removed = false;

        if (d_insertedChip)
        {
            unsigned int currentWait = 0;
            do
            {
                std::shared_ptr<Chip> chip = getChipInAir();
                if (!chip)
                {
                    d_insertedChip.reset();
                    removed = true;
                }

                if (!removed)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    currentWait += 100;
                }
            } while (!removed && (maxwait == 0 || currentWait < maxwait));
        }

        return removed;
    }

    bool IdOnDemandReaderUnit::connect()
    {
        return bool(d_insertedChip);
    }

    void IdOnDemandReaderUnit::disconnect()
    {
    }

    std::shared_ptr<Chip> IdOnDemandReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;

        // Change the reader state, but no other function to achieve the card detection...
        if (read())
        {
			chip = createChip(CHIP_GENERICTAG);
        }

        return chip;
    }

    std::shared_ptr<Chip> IdOnDemandReaderUnit::createChip(std::string type)
    {
        std::shared_ptr<Chip> chip;
        std::shared_ptr<ReaderCardAdapter> rca;
		if (type == CHIP_GENERICTAG)
        {
            chip.reset(new GenericTagIdOnDemandChip());
            std::shared_ptr<Commands> commands(new GenericTagIdOnDemandCommands());
            commands->setChip(chip);
            chip->setCommands(commands);
            rca = getDefaultReaderCardAdapter();
            rca->setDataTransport(getDataTransport());
            commands->setReaderCardAdapter(rca);
        }
        else
        {
            chip = ReaderUnit::createChip(type);
        }

        return chip;
    }

    std::shared_ptr<Chip> IdOnDemandReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > IdOnDemandReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<IdOnDemandReaderCardAdapter> IdOnDemandReaderUnit::getDefaultIdOnDemandReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<IdOnDemandReaderCardAdapter>(adapter);
    }

    string IdOnDemandReaderUnit::getReaderSerialNumber()
    {
        string ret;

        return ret;
    }

    bool IdOnDemandReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    bool IdOnDemandReaderUnit::connectToReader()
    {
        LOG(LogLevel::INFOS) << "Connecting to reader...";
        bool ret = getDataTransport()->connect();
        if (ret)
        {
            authenticateSDK();
        }

        return ret;
    }

    void IdOnDemandReaderUnit::disconnectFromReader()
    {
        LOG(LogLevel::INFOS) << "Disconnecting from reader...";

        // Close the session with the reader
        //authenticateSDK(0000);	 // A wrong authenticate code logout the session
        getDataTransport()->disconnect();
    }

    std::vector<unsigned char> IdOnDemandReaderUnit::getPingCommand() const
    {
        std::string strcmd = "BEEP";
        return std::vector<unsigned char>(strcmd.begin(), strcmd.end());
    }

    void IdOnDemandReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void IdOnDemandReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<IdOnDemandReaderProvider> IdOnDemandReaderUnit::getIdOnDemandReaderProvider() const
    {
        return std::dynamic_pointer_cast<IdOnDemandReaderProvider>(getReaderProvider());
    }
}
