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
#include "mifarecardprovider.hpp"
#include "mifareultralightcardprovider.hpp"
#include "desfirecardprovider.hpp"
#include "desfireev1cardprovider.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "commands/mifarerplethcommands.hpp"
#include "commands/mifareultralightrplethcommands.hpp"
#include <boost/filesystem.hpp>

namespace logicalaccess
{
	RplethReaderUnit::RplethReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new RplethReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new RplethReaderCardAdapter());
		d_card_type = "UNKNOWN";

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
						Sleep(250);
		#elif defined(LINUX)
						usleep(250000);
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
		return (d_insertedChip);
	}

	void RplethReaderUnit::disconnect()
	{
	}

	boost::shared_ptr<Chip> RplethReaderUnit::getChipInAir(unsigned int maxwait)
	{
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf;
		if (getRplethConfiguration()->getType() == ReaderType::WIEGAND)
		{
			buf = badge(maxwait);
			if (buf.size() > 0)
			{
				chip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
				chip->setChipIdentifier(AsciiToHex (buf));
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
					if (buf[1] == ChipType::MIFARE)
					{
						chip = createChip ("Mifare");
						buf.erase (buf.begin(), buf.begin()+2);
						chip->setChipIdentifier(AsciiToHex (buf));
					}
					else if (buf[1] == ChipType::DESFIRE)
					{
						chip = createChip ("DESFire");
						buf.erase (buf.begin(), buf.begin()+2);
						chip->setChipIdentifier(AsciiToHex (buf));
					}
					else if (buf[1] == ChipType::MIFAREULTRALIGHT)
					{
						chip = createChip ("MifareUltralight");
						buf.erase (buf.begin(), buf.begin()+2);
						chip->setChipIdentifier(AsciiToHex (buf));
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
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;
			boost::shared_ptr<Commands> commands;

			if (type ==  "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				cp.reset(new MifareCardProvider());
				commands.reset(new MifareRplethCommands);
				rca = getDefaultReaderCardAdapter();
			}
			else if (type ==  "MifareUltralight")
			{
				cp.reset(new MifareUltralightCardProvider());
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
				cp.reset(new DESFireCardProvider());
				commands.reset(new DESFireISO7816Commands());
				boost::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()), chip->getChipIdentifier());
			}
			else
			{
				return chip;
			}
			rca->setReaderUnit(shared_from_this());
			if (commands)
			{
				commands->setReaderCardAdapter(rca);
			}
			if (cp)
			{
				cp->setCommands(commands);
			}
			else
			{
				cp = LibraryManager::getInstance()->getCardProvider(type);
			}
			chip->setCardProvider(cp);
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
		if (!d_socket)
		{
			boost::asio::ip::tcp::endpoint endpoint;
			endpoint.address(boost::asio::ip::address::from_string(getRplethConfiguration()->getReaderAddress()));
			endpoint.port(getRplethConfiguration()->getPort());
			
			d_socket.reset(new boost::asio::ip::tcp::socket (ios));

			try
			{
				d_socket->connect(endpoint);
			}
			catch(boost::system::system_error& ex)
			{
				ERROR_("Socket connection error: %s", ex.what());
				d_socket.reset();
			}
		}
		return (d_socket);
	}

	void RplethReaderUnit::disconnectFromReader()
	{
		if (d_socket->is_open())
		{
			d_socket->shutdown(d_socket->shutdown_both);
			d_socket->close();
		}
		if (d_socket)
			d_socket.reset();
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
		answer = getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
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
		getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethReaderUnit::setReaderIp(std::vector<unsigned char> address)
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
			getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		}
	}

	void RplethReaderUnit::setReaderMac(std::vector<unsigned char> address)
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
			getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		}
	}
	
	void RplethReaderUnit::setReaderSubnet(std::vector<unsigned char> address)
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
			getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
		}
	}
	
	void RplethReaderUnit::setReaderGateway(std::vector<unsigned char> address)
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
			getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
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
		getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethReaderUnit::resetReader()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::RESET));
		command.push_back (static_cast<unsigned char>(0x00));
		getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
	}

	void RplethReaderUnit::setReaderMessage(std::string message)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::MESSAGE));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		getDefaultRplethReaderCardAdapter()->sendCommand (command, 0);
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
		getDefaultRplethReaderCardAdapter()->sendCommand(command, 0);
		res = receiveBadge(timeout);
		return res;
	}

	std::vector<unsigned char> RplethReaderUnit::receiveBadge (long int timeout)
	{
		std::vector<unsigned char> res;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(Device::HID));
		cmd.push_back(static_cast<unsigned char>(HidCommand::BADGE));
		res = getDefaultRplethReaderCardAdapter()->receiveAnwser(cmd, timeout);
		// res contains full wiegand trame, it need to take the csn
		res = getCsn (res);
		return res;
	}

	std::vector<unsigned char> RplethReaderUnit::getCsn (std::vector<unsigned char> trame)
	{
		std::vector<unsigned char> result;
		boost::shared_ptr<RplethReaderUnitConfiguration> conf = boost::dynamic_pointer_cast<RplethReaderUnitConfiguration>(d_readerUnitConfig);
		if (conf->getLength() != 0)
		{
			for (int i = 0; i < conf->getOffset(); i++)
			{
				trame.pop_back ();
			}
			result.insert (result.begin(), trame.begin()+(trame.size() - conf->getLength()), trame.end());
		}
		else
		{
			result.insert (result.begin(), trame.begin(), trame.end());
		}
		return result;
	}

	std::vector<unsigned char> RplethReaderUnit::AsciiToHex (std::vector<unsigned char> source)
	{
		std::vector<unsigned char> res;
		char tmp [3];
		for (size_t i = 0; i < source.size ()-2; i+=2)
		{
			tmp [0] = source[i];
			tmp [1] = source[i+1];
			tmp [2] = '\0';
			res.push_back (static_cast<unsigned char>(strtoul (tmp, NULL, 16)));
		}
		return res;
	}
}
