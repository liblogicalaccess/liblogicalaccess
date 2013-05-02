/**
 * \file RplethReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Rpleth reader unit.
 */

#include "RplethReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "RplethReaderProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/Chip.h"
#include "ReaderCardAdapters/RplethReaderCardAdapter.h"
#include <boost/filesystem.hpp>

namespace LOGICALACCESS
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
		unsigned int currentWait = 0;

		do
		{
			EXCEPTION_ASSERT_WITH_LOG(false, std::invalid_argument, "Fuck");
			boost::shared_ptr<Chip> chip = getChipInAir();
			std::cout << "insertion" << std::endl;
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

	bool RplethReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;
		std::vector<unsigned char> badge_number;

		if (d_insertedChip)
		{
			do
			{
				boost::shared_ptr<Chip> chip = getChipInAir();
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

	bool RplethReaderUnit::connect()
	{
		return (d_insertedChip);
	}

	void RplethReaderUnit::disconnect()
	{
	}

	boost::shared_ptr<Chip> RplethReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;

		std::cout << "GetChipInAir" << std::endl;
		std::vector<unsigned char> buf = badge();
		if (buf.size() > 0)
		{

			chip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
			chip->setChipIdentifier(buf);
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

			if (type == "GenericTag")
				rca = getDefaultReaderCardAdapter();
			else
				return chip;

			rca->setReaderUnit(shared_from_this());
			if(cp)
			{
				chip->setCardProvider(cp);
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
			boost::asio::io_service ios;
			boost::asio::ip::tcp::endpoint endpoint;
			endpoint.address(boost::asio::ip::address::from_string(getRplethConfiguration()->getReaderAddress()));
			endpoint.port(getRplethConfiguration()->getPort());
			d_socket.reset(new boost::asio::ip::tcp::socket(ios));
			try
			{
				d_socket->connect(endpoint);
			}
			catch(boost::system::system_error&)
			{
				d_socket.reset();
			}
		}

		return (d_socket);
	}

	void RplethReaderUnit::disconnectFromReader()
	{
		if (d_socket)
		{
			d_socket->close();
			d_socket.reset();
		}
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
		answer = d_defaultReaderCardAdapter->sendCommand (command, 0);
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
		d_defaultReaderCardAdapter->sendCommand (command, 0);
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
			d_defaultReaderCardAdapter->sendCommand (command, 0);
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
			d_defaultReaderCardAdapter->sendCommand (command, 0);
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
			d_defaultReaderCardAdapter->sendCommand (command, 0);
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
			d_defaultReaderCardAdapter->sendCommand (command, 0);
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
		d_defaultReaderCardAdapter->sendCommand (command, 0);
	}

	void RplethReaderUnit::setConfWiegand(unsigned char offset, unsigned char lenght, unsigned char tramSize)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::WIEGAND));
		command.push_back (static_cast<unsigned char>(0x03));
		command.push_back (static_cast<unsigned char>(offset));
		command.push_back (static_cast<unsigned char>(lenght));
		command.push_back (static_cast<unsigned char>(tramSize));
		d_defaultReaderCardAdapter->sendCommand (command, 0);
	}

	void RplethReaderUnit::resetReader()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::RESET));
		command.push_back (static_cast<unsigned char>(0x00));
		d_defaultReaderCardAdapter->sendCommand (command, 0);
	}

	void RplethReaderUnit::setReaderMessage(std::string message)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::MESSAGE));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		d_defaultReaderCardAdapter->sendCommand (command, 0);
	}

	void RplethReaderUnit::nop()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::NOP));
		command.push_back (static_cast<unsigned char>(0x00));
		d_defaultReaderCardAdapter->sendCommand (command, 0);
	}

	std::vector<unsigned char> RplethReaderUnit::badge(long int timeout)
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> res;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::BADGE));
		command.push_back (static_cast<unsigned char>(0x00));
		d_defaultReaderCardAdapter->sendCommand(command, 0);
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
		return res;
	}
}
