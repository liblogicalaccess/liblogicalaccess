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
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "commands/generictagidondemandcommands.hpp"

namespace logicalaccess
{
	IdOnDemandReaderUnit::IdOnDemandReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new IdOnDemandReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<IdOnDemandReaderCardAdapter> (new IdOnDemandReaderCardAdapter()));
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/IdOnDemandReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
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
		INFO_("Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	void IdOnDemandReaderUnit::authenticateSDK()
	{
		authenticateSDK(getIdOnDemandConfiguration()->getAuthCode());
	}

	void IdOnDemandReaderUnit::authenticateSDK(std::string authCode)
	{
		char cmd[64];
#ifndef __linux__
		sprintf_s(cmd, sizeof(cmd), "AUTH %s", authCode.c_str());
#else
		sprintf(cmd, "AUTH %s", authCode.c_str());
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
		catch(CardException& e)
		{
			ERROR_("Exception Read {%s}", e.what());
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
		catch(CardException& e)
		{
			ERROR_("Exception Verify {%s}", e.what());
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
		catch(CardException& e)
		{
			ERROR_("Exception Write {%s}", e.what());
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
			boost::shared_ptr<Chip> chip = getChipInAir();
			if (chip)
			{
				d_insertedChip = chip;
				inserted = true;
			}

			if (!inserted)
			{
#ifdef _WINDOWS
				Sleep(100);
#elif defined(LINUX)
				usleep(100000);
#endif
				currentWait += 100;
			}
		} while (!inserted && (maxwait == 0 || currentWait < maxwait));

		return inserted;
	}

	bool IdOnDemandReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if (d_insertedChip)
		{
			do
			{
				boost::shared_ptr<Chip> chip = getChipInAir();
				if (!chip)
				{
					d_insertedChip.reset();
					removed = true;
				}

				if (!removed)
				{
	#ifdef _WINDOWS
					Sleep(100);
	#elif defined(LINUX)
					usleep(100000);
	#endif
					currentWait += 100;
				}
			} while (!removed && (maxwait == 0 || currentWait < maxwait));
		}

		return removed;
	}

	bool IdOnDemandReaderUnit::connect()
	{
		return (d_insertedChip);
	}

	void IdOnDemandReaderUnit::disconnect()
	{
	}

	boost::shared_ptr<Chip> IdOnDemandReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;

		// Change the reader state, but no other function to achieve the card detection...
		if (read())
		{
			chip = createChip("GenericTag");
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> IdOnDemandReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip;
		boost::shared_ptr<ReaderCardAdapter> rca;
		if (type == "GenericTag")
		{
			chip.reset(new GenericTagIdOnDemandChip());
			boost::shared_ptr<Commands> commands(new GenericTagIdOnDemandCommands());
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

	boost::shared_ptr<Chip> IdOnDemandReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > IdOnDemandReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<IdOnDemandReaderCardAdapter> IdOnDemandReaderUnit::getDefaultIdOnDemandReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<IdOnDemandReaderCardAdapter>(adapter);
	}

	string IdOnDemandReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool IdOnDemandReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool IdOnDemandReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Connecting to reader...");
		bool ret = getDataTransport()->connect();
		if (ret)
		{
			authenticateSDK();
		}

		return ret;
	}

	void IdOnDemandReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Disconnecting from reader...");

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

	boost::shared_ptr<IdOnDemandReaderProvider> IdOnDemandReaderUnit::getIdOnDemandReaderProvider() const
	{
		return boost::dynamic_pointer_cast<IdOnDemandReaderProvider>(getReaderProvider());
	}
}
