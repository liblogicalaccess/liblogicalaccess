/**
 * \file rplethreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader unit.
 */

#include "rplethreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "rplethreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/rplethreadercardadapter.hpp"
#include "readercardadapters/iso7816rplethreadercardadapter.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "commands/mifarerplethcommands.hpp"
#include "commands/mifareultralightrplethcommands.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include "rplethledbuzzerdisplay.hpp"
#include "rplethlcddisplay.hpp"
#include "rplethdatatransport.hpp"

namespace logicalaccess
{
	RplethReaderUnit::RplethReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new RplethReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<RplethReaderCardAdapter> (new RplethReaderCardAdapter()));
		
		boost::shared_ptr<RplethDataTransport> dataTransport(new RplethDataTransport());
		dataTransport->setIpAddress("192.168.1.100");
		dataTransport->setPort(23);
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";
		d_lcdDisplay.reset(new RplethLCDDisplay());
		d_ledBuzzerDisplay.reset(new RplethLEDBuzzerDisplay());

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/RplethReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	RplethReaderUnit::~RplethReaderUnit()
	{
		disconnectFromReader();
	}

	std::string RplethReaderUnit::getName() const
	{
		string ret = "";
		return ret;
	}

	std::string RplethReaderUnit::getConnectedName()
	{
		return getName();
	}

	void RplethReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool RplethReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		boost::shared_ptr<Chip> chip = getChipInAir(maxwait);
		if (chip)
		{
			d_insertedChip = chip;
			inserted = true;
		}

		return inserted;
	}

	bool RplethReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;
		if (d_insertedChip)
		{
			while (!removed && ((currentWait < maxwait) || maxwait == 0))
			{
				boost::shared_ptr<Chip> chip = getChipInAir(250);
				if (chip)
				{
					if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
					{
						d_insertedChip.reset();
						removed = true;
					}
					else
					{
		#ifdef _WINDOWS
						Sleep(100);
		#elif defined(LINUX)
						usleep(100000);
		#endif
						currentWait += 250;
					}

				}
				else
				{
					d_insertedChip.reset();
					removed = true;
				}
			}
		}
		return removed;
	}

	bool RplethReaderUnit::connect()
	{
		INFO_SIMPLE_("Starting connect...");
		if (getSingleChip())
		{
			if (getSingleChip()->getCardType() == "DESFire")
			{
				std::vector<unsigned char> tmp = rats();
				if (tmp.size() == 0)
				{
					d_insertedChip = getChipInAir();
					tmp = rats();
					if (tmp.size() == 0)
					{
						d_insertedChip.reset();
					}
				}
			}
		}

		return bool(d_insertedChip);
	}

	void RplethReaderUnit::disconnect()
	{
		/*
		if (getRplethConfiguration()->getMode() == RplethMode::RS)
		{
			getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("x");
			getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("v");
		}*/

		INFO_SIMPLE_("Disconnected from the chip");
	}

	boost::shared_ptr<Chip> RplethReaderUnit::getChipInAir(unsigned int maxwait)
	{
		INFO_SIMPLE_("Starting get chip in air...");

		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf;
		if (getRplethConfiguration()->getMode() == RplethMode::WIEGAND)
		{
			buf = badge(maxwait);
			if (buf.size() > 0)
			{
				chip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
				chip->setChipIdentifier(buf);
			}
		}
		else
		{
			unsigned int currentWait = 0;
			while (!chip && (maxwait == 0 || currentWait < maxwait))
			{
				buf = getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("s");
				if (buf.size () > 1)
				{
					buf = asciiToHex (buf);
					if (buf[0] == ChipType::MIFARE)
					{
						chip = createChip ("Mifare");
						buf.erase (buf.begin());
						chip->setChipIdentifier(buf);
					}
					else if (buf[0] == ChipType::DESFIRE)
					{
						chip = createChip ("DESFire");
						buf.erase (buf.begin());
						chip->setChipIdentifier(buf);
					}
					else if (buf[0] == ChipType::MIFAREULTRALIGHT)
					{
						chip = createChip ("MifareUltralight");
						buf.erase (buf.begin());
						chip->setChipIdentifier(buf);
					}
				}
				if (!chip)
				{
		#ifdef _WINDOWS
					Sleep(250);
		#elif defined(LINUX)
					usleep(250000);
		#endif
					currentWait += 250;
				}
			}
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> RplethReaderUnit::createChip(std::string type)
	{
		INFO_("Create chip %s", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type ==  "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				commands.reset(new MifareRplethCommands);
				rca = getDefaultReaderCardAdapter();
			}
			else if (type ==  "MifareUltralight")
			{
				commands.reset(new MifareUltralightRplethCommands());
				rca = getDefaultReaderCardAdapter();
			}
			else if (type == "GenericTag")
			{
				rca = getDefaultReaderCardAdapter();
			}
			else if (type == "DESFire")
			{
				rca.reset(new ISO7816RplethReaderCardAdapter());
				commands.reset(new DESFireISO7816Commands());
				boost::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()), chip->getChipIdentifier());
			}

			if (rca)
			{
				rca->setDataTransport(getDataTransport());
				if (commands)
				{
					commands->setReaderCardAdapter(rca);
					commands->setChip(chip);
					chip->setCommands(commands);
				}
			}
		}
		return chip;
	}

	boost::shared_ptr<Chip> RplethReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > RplethReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<RplethReaderCardAdapter> RplethReaderUnit::getDefaultRplethReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		if (!adapter->getDataTransport())
			adapter->setDataTransport(getDataTransport());
		return boost::dynamic_pointer_cast<RplethReaderCardAdapter>(adapter);
	}

	string RplethReaderUnit::getReaderSerialNumber()
	{
		return "";
	}

	bool RplethReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool RplethReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Starting connection to reader...");
		boost::shared_ptr<DataTransport> dataTransport = getDataTransport();
		if (!dataTransport->getReaderUnit())
		{
			dataTransport->setReaderUnit(shared_from_this());
			setDataTransport(dataTransport);
		}
		return getDataTransport()->connect();
	}

	void RplethReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Starting disconnection to reader...");
		getDataTransport()->disconnect();
	}

	void RplethReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		
		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RplethReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<RplethReaderProvider> RplethReaderUnit::getRplethReaderProvider() const
	{
		return boost::dynamic_pointer_cast<RplethReaderProvider>(getReaderProvider());
	}

	boost::shared_ptr<boost::asio::ip::tcp::socket> RplethReaderUnit::getSocket()
	{
		return d_socket;
	}

	bool RplethReaderUnit::getDhcpState ()
	{
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::RPLETH));
		command.push_back(static_cast<unsigned char>(RplethCommand::STATEDHCP));
		command.push_back(static_cast<unsigned char>(0x00));
		answer = getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
		if (answer.size() > 0)
			res = answer[0] != 0x00;
		return res;
	}

	void RplethReaderUnit::setDhcpState (bool status)
	{
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(Device::RPLETH));
		command.push_back(static_cast<unsigned char>(RplethCommand::DHCP));
		command.push_back(static_cast<unsigned char>(0x01));
		command.push_back(static_cast<unsigned char>(status));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
	}

	void RplethReaderUnit::setReaderIp(const std::vector<unsigned char>& address)
	{
		if (address.size() == 4)
		{
			std::vector<unsigned char> command;
			command.push_back(static_cast<unsigned char>(Device::RPLETH));
			command.push_back(static_cast<unsigned char>(RplethCommand::IP));
			command.push_back(static_cast<unsigned char>(0x04));
			for (int i = 0; i < 4; i++)
			{
				command.push_back(static_cast<unsigned char>(address[i]));
			}
			getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
		}
	}

	void RplethReaderUnit::setReaderMac(const std::vector<unsigned char>& address)
	{
		if (address.size() == 4)
		{
			std::vector<unsigned char> command;
			command.push_back(static_cast<unsigned char>(Device::RPLETH));
			command.push_back(static_cast<unsigned char>(RplethCommand::MAC));
			command.push_back(static_cast<unsigned char>(0x04));
			for (int i = 0; i < 4; i++)
			{
				command.push_back(static_cast<unsigned char>(address[i]));
			}
			getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
		}
	}
	
	void RplethReaderUnit::setReaderSubnet(const std::vector<unsigned char>& address)
	{
		if (address.size() == 4)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::RPLETH));
			command.push_back (static_cast<unsigned char>(RplethCommand::SUBNET));
			command.push_back (static_cast<unsigned char>(0x04));
			for (int i = 0; i < 4; i++)
			{
				command.push_back (static_cast<unsigned char>(address[i]));
			}
			getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
		}
	}
	
	void RplethReaderUnit::setReaderGateway(const std::vector<unsigned char>& address)
	{
		if (address.size() == 4)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::RPLETH));
			command.push_back (static_cast<unsigned char>(RplethCommand::GATEWAY));
			command.push_back (static_cast<unsigned char>(0x04));
			for (int i = 0; i < 4; i++)
			{
				command.push_back (static_cast<unsigned char>(address[i]));
			}
			getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
		}
	}

	void RplethReaderUnit::setReaderPort(int port)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::PORT));
		command.push_back (static_cast<unsigned char>(0x02));
		command.push_back (static_cast<unsigned char>(port << 8));
		command.push_back (static_cast<unsigned char>(port & 0xff));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
	}

	void RplethReaderUnit::resetReader()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::RESET));
		command.push_back (static_cast<unsigned char>(0x00));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
	}

	void RplethReaderUnit::setReaderMessage(const std::string& message)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::MESSAGE));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		getDefaultRplethReaderCardAdapter()->sendRplethCommand (command, 0);
	}

	void RplethReaderUnit::nop()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::NOP));
		command.push_back (static_cast<unsigned char>(0x00));
		getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	std::vector<unsigned char> RplethReaderUnit::badge(long int timeout)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> res;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::BADGE));
		command.push_back (static_cast<unsigned char>(0x00));
		try
		{
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, 0);
			res = receiveBadge(timeout);
		}
		catch(std::invalid_argument& e)
		{
			std::cout << e.what() << std::endl;
		}
		return res;
	}

	std::vector<unsigned char> RplethReaderUnit::receiveBadge (long int timeout)
	{
		COM_SIMPLE_("receiveBadge");
		std::vector<unsigned char> res;
		std::vector<unsigned char> cmd;
		long int currentWait = 0;
		try
		{
			res = getDefaultRplethReaderCardAdapter()->sendRplethCommand(cmd, timeout);
			// res contains full wiegand trame, it need to take the csn
			if (res.size() > 0)
				res = getCsn (res);
		}
		catch (std::invalid_argument&)
		{

		}
		return res;
	}

	std::vector<unsigned char> RplethReaderUnit::getCsn (const std::vector<unsigned char>& trame)
	{
		std::vector<unsigned char> result;
		long long tmp = 0;
		boost::shared_ptr<RplethReaderUnitConfiguration> conf = boost::dynamic_pointer_cast<RplethReaderUnitConfiguration>(d_readerUnitConfig);
		if (conf->getLength() != 0)
		{
			if (trame.size()*8 >= static_cast<size_t>(conf->getLength() + conf->getOffset()))
			{
				for (int i = 0; i < static_cast<int>(trame.size()); i++)
				{
					tmp <<= 8;
					tmp |= trame[i];
				}
				tmp >>= conf->getOffset();
				tmp &= static_cast<int>(pow(2, conf->getLength())-1);
				int size = 0;
				if (conf->getLength()%8==0)
					size = conf->getLength()/8;
				else
					size = conf->getLength()/8+1;
				for (int i = 0, j = 0; i < size; i++, j += 8)
				{
					result.insert(result.begin(), (tmp >> j) & 0xff);
				}
			}
			else
			{
				WARNING_("Wrong trame length (%d < (%d + %d))", trame.size()*8, conf->getLength(), conf->getOffset());
			}
		}
		else
		{
			result.insert (result.begin(), trame.begin(), trame.end());
		}

		return result;
	}

	std::vector<unsigned char> RplethReaderUnit::asciiToHex (const std::vector<unsigned char>& source)
	{
		std::vector<unsigned char> res;
		char tmp [3];
		if (source.size() > 1)
		{
			for (size_t i = 0; i < source.size() - 2; i+=2)
			{
				tmp [0] = source[i];
				if (source.size()%2 == 1 && i == source.size()-1)
					tmp [1] = '\0';
				else
					tmp [1] = source[i+1];
				tmp [2] = '\0';
				res.push_back (static_cast<unsigned char>(strtoul (tmp, NULL, 16)));
			}
		}
		return res;
	}

	std::vector<unsigned char> RplethReaderUnit::reqA ()
	{
		std::vector<unsigned char> answer = getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("t01E326");
		answer = asciiToHex (answer);
		if (answer.size () > 1)
		{
			if (answer[0] == answer.size()-1)
			{
				answer.erase (answer.begin());
			}
			else
				answer.clear();
		}
		else
		{
			answer.clear();
		}
		return answer;
	}

	std::vector<unsigned char> RplethReaderUnit::rats ()
	{
		INFO_("Sending a RATS");
		std::vector<unsigned char> answer = getDefaultRplethReaderCardAdapter()->sendAsciiCommand ("t020FE020");
		INFO_("Answer rats %s", BufferHelper::getHex(answer).c_str());
		answer = asciiToHex (answer);
		INFO_("Answer rats %s", BufferHelper::getHex(answer).c_str());
		if (answer.size () > 1)
		{
			if (answer[0] == answer.size())
			{
				answer.erase (answer.begin());
			}
			else
				answer.clear();
		}
		else
		{
			answer.clear();
		}
		return answer;
	}
}
