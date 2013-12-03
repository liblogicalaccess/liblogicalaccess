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
#include <boost/filesystem.hpp>
#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include "rplethledbuzzerdisplay.hpp"
#include "rplethlcddisplay.hpp"
#include "rplethdatatransport.hpp"
#include "desfirechip.hpp"
#include "commands/desfireiso7816commands.hpp"

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
		if (getDefaultRplethReaderCardAdapter()->getDataTransport()->isConnected())
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::HID));
			command.push_back (static_cast<unsigned char>(HidCommand::SET_CARDTYPE));
			command.push_back (static_cast<unsigned char>(cardType.size()));
			command.insert(command.end(), cardType.begin(), cardType.end());
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
		}
	}

	bool RplethReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		if (getRplethConfiguration()->getMode() == RplethMode::PROXY)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::HID));
			command.push_back (static_cast<unsigned char>(HidCommand::WAIT_INSERTION));
			command.push_back (static_cast<unsigned char>(0x01));
			command.push_back (static_cast<unsigned char>(maxwait));
			std::vector<unsigned char> answer;
			try
			{
				answer = getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, maxwait);
			}
			catch(LibLogicalAccessException&)
			{
				answer.clear();
			}

			if (answer.size() > 0)
			{
				unsigned char ctypelen = answer[0];
				EXCEPTION_ASSERT_WITH_LOG(answer.size() >= static_cast<size_t>(1 + ctypelen + 1), LibLogicalAccessException, "Wrong card type length.");
				std::string ctype = (d_card_type == "UNKNOWN") ? std::string(answer.begin() + 1, answer.begin() + 1 + ctypelen) : d_card_type;
				unsigned char csnlen = answer[1 + ctypelen];
				EXCEPTION_ASSERT_WITH_LOG(answer.size() >= static_cast<size_t>(1 + ctypelen + 1 + csnlen), LibLogicalAccessException, "Wrong csn length.");
				std::vector<unsigned char> csn = std::vector<unsigned char>(answer.begin() + 1 + ctypelen + 1, answer.end());

				d_insertedChip = d_proxyReader->createChip(ctype, csn);
				inserted = bool(d_insertedChip);
			}
		}
		else
		{
			if (removalIdentifier.size() > 0)
			{
				d_insertedChip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
				d_insertedChip->setChipIdentifier(removalIdentifier);
				removalIdentifier.clear();
				inserted = true;
			}
			else
			{
				try
				{
					boost::shared_ptr<Chip> chip = getChipInAir(maxwait);
					if (chip)
					{
						d_insertedChip = chip;
						inserted = true;
					}
				}
				catch (LibLogicalAccessException&)
				{}
			}
		}

		return inserted;
	}

	bool RplethReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		if (d_insertedChip)
		{
			if (getRplethConfiguration()->getMode() == RplethMode::PROXY)
			{
				std::vector<unsigned char> command;
				command.push_back (static_cast<unsigned char>(Device::HID));
				command.push_back (static_cast<unsigned char>(HidCommand::WAIT_REMOVAL));
				command.push_back (static_cast<unsigned char>(0x01));
				command.push_back (static_cast<unsigned char>(maxwait));
				std::vector<unsigned char> answer;
				try
				{
					getDefaultRplethReaderCardAdapter()->sendRplethCommand(command, maxwait);
					d_insertedChip.reset();
					removed = true;
				}
				catch(LibLogicalAccessException&)
				{
				}
			}
			else
			{
				unsigned int currentWait = 0;
				removalIdentifier.clear();
			
				while (!removed && ((currentWait < maxwait) || maxwait == 0))
				{
					boost::shared_ptr<Chip> chip;
					try
					{
						chip = getChipInAir(250);
					}
					catch (LibLogicalAccessException&)
					{}
					if (chip)
					{
						std::vector<unsigned char> tmpId = chip->getChipIdentifier();
						if (tmpId != d_insertedChip->getChipIdentifier())
						{
							d_insertedChip.reset();
							removalIdentifier = tmpId;
							removed = true;
						}
						else
						{
			#ifdef _WINDOWS
							Sleep(100);
			#elif defined(__unix__)
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
		}

		return removed;
	}

	bool RplethReaderUnit::connect()
	{
		if (getRplethConfiguration()->getMode() == RplethMode::PROXY)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::HID));
			command.push_back (static_cast<unsigned char>(HidCommand::CONNECT));
			command.push_back (static_cast<unsigned char>(0x00));
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);

			if (d_insertedChip->getChipIdentifier().size() == 0)
			{
				d_insertedChip->setChipIdentifier(getInsertedChipIdentifier());
			}

			if (d_insertedChip->getGenericCardType() == "DESFire")
					boost::dynamic_pointer_cast<DESFireISO7816Commands>(d_insertedChip->getCommands())->getCrypto()->setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(d_insertedChip->getProfile()), d_insertedChip->getChipIdentifier());
		}

		return true;
	}

	void RplethReaderUnit::disconnect()
	{
		if (getRplethConfiguration()->getMode() == RplethMode::PROXY)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::HID));
			command.push_back (static_cast<unsigned char>(HidCommand::DISCONNECT));
			command.push_back (static_cast<unsigned char>(0x00));
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
		}

		INFO_SIMPLE_("Disconnected from the chip");
	}

	std::vector<unsigned char> RplethReaderUnit::getInsertedChipIdentifier()
	{
		std::vector<unsigned char> csn;
		if (getRplethConfiguration()->getMode() == RplethMode::PROXY)
		{
			std::vector<unsigned char> command;
			command.push_back (static_cast<unsigned char>(Device::HID));
			command.push_back (static_cast<unsigned char>(HidCommand::GET_CSN));
			command.push_back (static_cast<unsigned char>(0x00));
			csn = getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
		}

		INFO_SIMPLE_("Inserted chip identifier get");
		return csn;
	}

	boost::shared_ptr<Chip> RplethReaderUnit::getChipInAir(unsigned int maxwait)
	{
		INFO_SIMPLE_("Starting get chip in air...");

		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf = receiveBadge(maxwait);
		if (buf.size() > 0)
		{
			chip = createChip((d_card_type == "UNKNOWN") ? "GenericTag" : d_card_type);
			chip->setChipIdentifier(buf);
		}
		else
		{
			chip = d_insertedChip;
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> RplethReaderUnit::createChip(std::string type)
	{
		INFO_("Create chip %s", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

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
		if (adapter)
		{
			if (!adapter->getDataTransport())
			{
				adapter->setDataTransport(getDataTransport());
			}
		}
		return boost::dynamic_pointer_cast<RplethReaderCardAdapter>(adapter);
	}

	string RplethReaderUnit::getReaderSerialNumber()
	{
		return "";
	}

	bool RplethReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
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
		
		bool connected = getDataTransport()->connect();
		if (connected && getRplethConfiguration()->getMode() == RplethMode::PROXY)
		{
			INFO_SIMPLE_("Data transport connected, initializing PROXY mode...");
			std::string type = getProxyReaderType();
			boost::shared_ptr<ReaderProvider> rp = LibraryManager::getInstance()->getReaderProvider(type);
			if (rp)
			{
				d_proxyReader = rp->createReaderUnit();
				boost::shared_ptr<RplethDataTransport> rpdt = boost::dynamic_pointer_cast<RplethDataTransport>(getDataTransport());

				EXCEPTION_ASSERT_WITH_LOG(rpdt, LibLogicalAccessException, "Rpleth data transport required for proxy mode.");

				boost::shared_ptr<RplethDataTransport> dataTransport(new RplethDataTransport());
				dataTransport->setIpAddress(rpdt->getIpAddress());
				dataTransport->setPort(rpdt->getPort());
				dataTransport->d_socket = rpdt->d_socket;
				d_proxyReader->setDataTransport(rpdt);
			}
		}

		return connected;
	}

	void RplethReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Starting disconnection to reader...");
		getDataTransport()->disconnect();
	}

	void RplethReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void RplethReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<RplethReaderProvider> RplethReaderUnit::getRplethReaderProvider() const
	{
		return boost::dynamic_pointer_cast<RplethReaderProvider>(getReaderProvider());
	}

	bool RplethReaderUnit::getDhcpState()
	{
		bool res = false;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(static_cast<unsigned char>(Device::RPLETH));
		command.push_back(static_cast<unsigned char>(RplethCommand::STATEDHCP));
		command.push_back(static_cast<unsigned char>(0x00));
		answer = getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
		if (answer.size() > 0)
			res = answer[0] != 0x00;
		return res;
	}

	void RplethReaderUnit::setDhcpState(bool status)
	{
		std::vector<unsigned char> command;
		command.push_back(static_cast<unsigned char>(Device::RPLETH));
		command.push_back(static_cast<unsigned char>(RplethCommand::DHCP));
		command.push_back(static_cast<unsigned char>(0x01));
		command.push_back(static_cast<unsigned char>(status));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
		getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
	}

	void RplethReaderUnit::resetReader()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::RESET));
		command.push_back (static_cast<unsigned char>(0x00));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
	}

	void RplethReaderUnit::setReaderMessage(const std::string& message)
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::RPLETH));
		command.push_back (static_cast<unsigned char>(RplethCommand::MESSAGE));
		command.push_back (static_cast<unsigned char>(message.size()));
		command.insert(command.end(), message.begin(), message.end());
		getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
	}

	void RplethReaderUnit::nop()
	{
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::NOP));
		command.push_back (static_cast<unsigned char>(0x00));
		getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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
			getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
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

	std::string RplethReaderUnit::getProxyReaderType()
	{
		std::string rptype = "";
		std::vector<unsigned char> command;
		command.push_back (static_cast<unsigned char>(Device::HID));
		command.push_back (static_cast<unsigned char>(HidCommand::GET_READERTYPE));
		command.push_back (static_cast<unsigned char>(0x00));
		std::vector<unsigned char> answer = getDefaultRplethReaderCardAdapter()->sendRplethCommand(command);
		if (answer.size() > 0)
		{
			rptype = std::string(answer.begin(), answer.end());
		}

		return rptype;
	}
}
