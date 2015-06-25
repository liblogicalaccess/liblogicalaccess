/**
 * \file deisterreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Deister reader unit.
 */

#include "deisterreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "deisterreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/deisterreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/deisterdatatransport.hpp"
#include "logicalaccess/myexception.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
    DeisterReaderUnit::DeisterReaderUnit()
        : ReaderUnit()
    {
        d_readerUnitConfig.reset(new DeisterReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<DeisterReaderCardAdapter>(new DeisterReaderCardAdapter()));
        std::shared_ptr<DeisterDataTransport> dataTransport(new DeisterDataTransport());
        setDataTransport(dataTransport);
        d_card_type = "UNKNOWN";

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/DeisterReaderUnit.config"), pt);
            d_card_type = pt.get("config.cardType", "UNKNOWN");
        }
        catch (...) {}
    }

    DeisterReaderUnit::~DeisterReaderUnit()
    {
        disconnectFromReader();
    }

    std::string DeisterReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string DeisterReaderUnit::getConnectedName()
    {
        return getName();
    }

    void DeisterReaderUnit::setCardType(std::string cardType)
    {
        d_card_type = cardType;
    }

    bool DeisterReaderUnit::waitInsertion(unsigned int maxwait)
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

    bool DeisterReaderUnit::waitRemoval(unsigned int maxwait)
    {
        bool removed = false;

        if (d_insertedChip)
        {
            std::chrono::steady_clock::time_point const clock_timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxwait);
            do
            {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //Deister 'forget' the card...
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

    bool DeisterReaderUnit::connect()
    {
        return true;
    }

    void DeisterReaderUnit::disconnect()
    {
    }

    bool DeisterReaderUnit::connectToReader()
    {
        getDataTransport()->setReaderUnit(shared_from_this());
        return getDataTransport()->connect();
    }

    void DeisterReaderUnit::disconnectFromReader()
    {
        getDataTransport()->disconnect();
    }

    std::vector<unsigned char> DeisterReaderUnit::getPingCommand() const
    {
        std::vector<unsigned char> cmd;

        cmd.push_back(0x0b);

        return cmd;
    }

    std::shared_ptr<Chip> DeisterReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;
        std::vector<unsigned char> cmd;
        cmd.push_back(static_cast<unsigned char>(0x0B));

        std::vector<unsigned char> pollBuf = getDefaultDeisterReaderCardAdapter()->sendCommand(cmd);
        if (pollBuf.size() > 0)
        {
            std::vector<unsigned char> tmpId;
            if (pollBuf.size() > 0)
            {
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[0] == 0x00, LibLogicalAccessException, "Bad polling answer, LOC byte");
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[1] == 0x00, LibLogicalAccessException, "Bad polling answer, RST byte");
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[2] == 0x01, LibLogicalAccessException, "Bad polling answer, NOB byte");
                // Only one card type supported at the same time for now
                std::string cardType = getCardTypeFromDeisterType(static_cast<DeisterCardType>(pollBuf[3]));
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[4] == 0x01, LibLogicalAccessException, "Bad polling answer, DT byte");
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[5] == 0x00, LibLogicalAccessException, "Bad polling answer, No byte");
                EXCEPTION_ASSERT_WITH_LOG(pollBuf[6] > 0, LibLogicalAccessException, "Bad polling answer, Size byte must be greater than zero");
                tmpId = std::vector<unsigned char>(pollBuf.begin() + 7, pollBuf.begin() + 7 + pollBuf[6]);
                std::reverse(tmpId.begin(), tmpId.end());
                chip = ReaderUnit::createChip(
                    (d_card_type == "UNKNOWN" ? cardType : d_card_type),
                    tmpId
                    );
            }
        }

        return chip;
    }

    std::string DeisterReaderUnit::getCardTypeFromDeisterType(DeisterCardType deisterCardType) const
    {
        switch (deisterCardType)
        {
        case DCT_MIFARE:
            return "Mifare";
        case DCT_MIFARE_ULTRALIGHT:
            return "MifareUltralight";
        case DCT_DESFIRE:
            return "DESFire";
        case DCT_ICODE1:
            return "iCode1";
        case DCT_STM_LRI_512:
            return "StmLri512";
        case DCT_ICODE2:
            return "iCode2";
        case DCT_INFINEON_MYD:
            return "InfineonMYD";
        case DCT_GENERIC_ISO15693:
            return "ISO15693";
        case DCT_TAGIT:
            return "GenericTag";
        case DCT_ICLASS:
            return "HIDiClass";
        case DCT_PROXLITE:
            return "ProxLite";
        case DCT_PROX:
            return "Prox";
        case DCT_EM4135:
            return "EM4135";
        case DCT_TAGIT_HFI:
            return "TagIt";
        case DCT_GENERIC_ISO14443B:
            return "GenericTag";
        case DCT_EM4102:
            return "EM4102";
        case DCT_SMARTFRAME:
            return "SmartFrame";

        default:
            return "UNKNOWN";
        }
    }

    std::shared_ptr<Chip> DeisterReaderUnit::createChip(std::string type)
    {
        std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

        if (chip)
        {
            std::shared_ptr<ReaderCardAdapter> rca;

            if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
                rca = getDefaultReaderCardAdapter();
            else
                return chip;

            rca->setDataTransport(getDataTransport());
        }
        return chip;
    }

    std::shared_ptr<Chip> DeisterReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > DeisterReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<DeisterReaderCardAdapter> DeisterReaderUnit::getDefaultDeisterReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<DeisterReaderCardAdapter>(adapter);
    }

    std::string DeisterReaderUnit::getReaderSerialNumber()
    {
        return std::string();
    }

    bool DeisterReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    void DeisterReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void DeisterReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<DeisterReaderProvider> DeisterReaderUnit::getDeisterReaderProvider() const
    {
        return std::dynamic_pointer_cast<DeisterReaderProvider>(getReaderProvider());
    }
}