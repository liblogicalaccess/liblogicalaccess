/**
 * \file gigatmsreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GIGA-TMS reader unit.
 */

#include "gigatmsreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "gigatmsreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/gigatmsreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "readercardadapters/gigatmsdatatransport.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
	GigaTMSReaderUnit::GigaTMSReaderUnit()
        : ReaderUnit(READER_GIGATMS)
    {
        d_readerUnitConfig.reset(new GigaTMSReaderUnitConfiguration());
	    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<GigaTMSReaderCardAdapter>());
        std::shared_ptr<GigaTMSDataTransport> dataTransport(new GigaTMSDataTransport());
		dataTransport->setPortBaudRate(19200);
	    ReaderUnit::setDataTransport(dataTransport);
		d_card_type = CHIP_UNKNOWN;

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/GigaTMSReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
        }
        catch (...) {}
    }

	GigaTMSReaderUnit::~GigaTMSReaderUnit()
    {
	    GigaTMSReaderUnit::disconnectFromReader();
    }

    std::string GigaTMSReaderUnit::getName() const
    {
        return getDataTransport()->getName();
    }

    std::string GigaTMSReaderUnit::getConnectedName()
    {
        return getName();
    }

    void GigaTMSReaderUnit::setCardType(std::string cardType)
    {
        d_card_type = cardType;
    }

    bool GigaTMSReaderUnit::waitInsertion(unsigned int maxwait)
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

    bool GigaTMSReaderUnit::waitRemoval(unsigned int maxwait)
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

    bool GigaTMSReaderUnit::connect()
    {
        return true;
    }

    void GigaTMSReaderUnit::disconnect()
    {
    }

	void GigaTMSReaderUnit::setCommunicationProtocol()
	{
		// For now only support ISO14443A
		ByteVector cmd;
		cmd.push_back(0x00);
		cmd.push_back(0x00);
		cmd.push_back(0x00);

		getDefaultGigaTMSReaderCardAdapter()->sendCommand(cmd);
	}

    std::shared_ptr<Chip> GigaTMSReaderUnit::getChipInAir()
    {
        std::shared_ptr<Chip> chip;
        ByteVector cmd;
        cmd.push_back(0x01);
		cmd.push_back(0x00);

        ByteVector rawidbuf = getDefaultGigaTMSReaderCardAdapter()->sendCommand(cmd);
        if (rawidbuf.size() > 0)
        {
            // The reader looks bugged and don't detect the card once on two time
            if (rawidbuf[0] == 0xE0)
            {
                rawidbuf = getDefaultGigaTMSReaderCardAdapter()->sendCommand(cmd);
            }

            if (rawidbuf[0] == 0x01)
            {
                std::string rawidstr = BufferHelper::getHex(ByteVector(rawidbuf.begin() + 1, rawidbuf.end()));
                ByteVector insertedId = formatHexString(rawidstr);
                chip = ReaderUnit::createChip(
					(d_card_type == CHIP_UNKNOWN ? CHIP_GENERICTAG : d_card_type),
                    insertedId
                    );
            }
        }

        return chip;
    }

    std::shared_ptr<Chip> GigaTMSReaderUnit::createChip(std::string type)
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

    std::shared_ptr<Chip> GigaTMSReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > GigaTMSReaderUnit::getChipList()
    {
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<GigaTMSReaderCardAdapter> GigaTMSReaderUnit::getDefaultGigaTMSReaderCardAdapter()
    {
        std::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
        return std::dynamic_pointer_cast<GigaTMSReaderCardAdapter>(adapter);
    }

    std::string GigaTMSReaderUnit::getReaderSerialNumber()
    {
        return std::string();
    }

    bool GigaTMSReaderUnit::isConnected()
    {
        return bool(d_insertedChip);
    }

    bool GigaTMSReaderUnit::connectToReader()
    {
		if (getDataTransport()->connect())
		{
			setCommunicationProtocol();
			return true;
		}

		return false;
    }

    void GigaTMSReaderUnit::disconnectFromReader()
    {
        getDataTransport()->disconnect();
    }

    void GigaTMSReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;
        ReaderUnit::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void GigaTMSReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        ReaderUnit::unSerialize(node);
    }

    std::shared_ptr<GigaTMSReaderProvider> GigaTMSReaderUnit::getGigaTMSReaderProvider() const
    {
        return std::dynamic_pointer_cast<GigaTMSReaderProvider>(getReaderProvider());
    }
}