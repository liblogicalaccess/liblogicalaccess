/**
 * \file smartidreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SmartID reader unit.
 */

#include "smartidreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "smartidreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "commands/mifaresmartidcommands.hpp"
#include "readercardadapters/smartidreadercardadapter.hpp"
#include "smartidledbuzzerdisplay.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{
	SmartIDReaderUnit::SmartIDReaderUnit()
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new SmartIDReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<SmartIDReaderCardAdapter> (new SmartIDReaderCardAdapter()));
		d_ledBuzzerDisplay.reset(new SmartIDLEDBuzzerDisplay());
		boost::shared_ptr<SerialPortDataTransport> dataTransport(new SerialPortDataTransport());
#ifndef _WINDOWS
		dataTransport->setPortBaudRate(B115200);
#else
		dataTransport->setPortBaudRate(CBR_115200);
#endif
		setDataTransport(dataTransport);
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/SmartIDReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }
	}

	SmartIDReaderUnit::~SmartIDReaderUnit()
	{
		disconnectFromReader();
	}

	std::string SmartIDReaderUnit::getName() const
	{
		return getDataTransport()->getName();
	}

	std::string SmartIDReaderUnit::getConnectedName()
	{
		return getName();
	}

	void SmartIDReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
		d_readerCommunication = getReaderCommunication(cardType);
		boost::dynamic_pointer_cast<ReaderCardAdapter>(d_readerCommunication)->setDataTransport(getDataTransport());
	}

	boost::shared_ptr<ReaderCommunication> SmartIDReaderUnit::getReaderCommunication(std::string cardType)
	{
		boost::shared_ptr<ReaderCommunication> ret;

		if (cardType == "Mifare1K" || cardType == "Mifare4K" || cardType == "Mifare")
			ret.reset(new MifareSmartIDReaderCardAdapter());
		else
			ret.reset(new SmartIDReaderCardAdapter());
		return ret;
	}

	bool SmartIDReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		do
		{
			try
			{
				d_readerCommunication->request();
				d_insertedChip = createChip(d_card_type);
				inserted = true;
			}
			catch (std::exception&)
			{
				inserted = false;
			}

			if (!inserted)
			{
#ifdef _WINDOWS
				Sleep(500);
#elif defined(__unix__)
				usleep(500000);
#endif
				currentWait += 500;
			}
		} while (!inserted && currentWait < maxwait);

		return inserted;
	}

	bool SmartIDReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		do
		{
			try
			{
				d_readerCommunication->request();
				removed = false;
			}
			catch (std::exception&)
			{
				removed = true;
			}

			if (!removed)
			{
#ifdef _WINDOWS
				Sleep(500);
#elif defined(__unix__)
				usleep(500000);
#endif
				currentWait += 500;
			}
		} while (!removed && currentWait < maxwait);

		if (removed)
		{
			d_insertedChip.reset();
		}

		return removed;
	}

	bool SmartIDReaderUnit::connect()
	{
		bool ret;
		std::vector<unsigned char> uid;

		resetRF();

		try
		{
			d_readerCommunication->request();
		}
		catch (std::exception&)
		{
		}

		try
		{
			uid = d_readerCommunication->anticollision();
			d_readerCommunication->selectIso(uid);

			d_insertedChip->setChipIdentifier(uid);

			ret = true;
		}
		catch (std::exception&)
		{
			ret = false;
		}

		return ret;
	}

	void SmartIDReaderUnit::disconnect()
	{
		resetRF();
	}

	std::vector<unsigned char> SmartIDReaderUnit::getCardSerialNumber()
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(0x01));

		std::vector<unsigned char> csn = getDefaultSmartIDReaderCardAdapter()->sendCommand(0x53, data);
		return csn;
	}

	boost::shared_ptr<Chip> SmartIDReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
		setcryptocontext setcryptocontextfct;

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<Commands> commands;

			if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				rca.reset(new MifareSmartIDReaderCardAdapter());
				commands.reset(new MifareSmartIDCommands());
			}
			else if (type == "DESFireEV1")
			{
				rca = getDefaultReaderCardAdapter();
				commands = LibraryManager::getInstance()->getCommands("DESFireEV1ISO7816");
				*(void**)(&setcryptocontextfct) = LibraryManager::getInstance()->getFctFromName("setCryptoContextDESFireEV1ISO7816Commands", LibraryManager::READERS_TYPE);
				setcryptocontextfct(&commands, &chip);
			}
			else if (type == "DESFire")
			{
				rca = getDefaultReaderCardAdapter();
				commands = LibraryManager::getInstance()->getCommands("DESFireISO7816");
				*(void**)(&setcryptocontextfct) = LibraryManager::getInstance()->getFctFromName("setCryptoContextDESFireEV1ISO7816Commands", LibraryManager::READERS_TYPE);
				setcryptocontextfct(&commands, &chip);
			}
			else if (type == "Twic")
			{
				rca = getDefaultReaderCardAdapter();
				commands = LibraryManager::getInstance()->getCommands("TwicISO7816");
			}
			else
				return chip;

			rca->setDataTransport(getDataTransport());
			if (commands)
			{
				commands->setReaderCardAdapter(rca);
				commands->setChip(chip);
				chip->setCommands(commands);
			}
		}
		return chip;
	}

	boost::shared_ptr<Chip> SmartIDReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > SmartIDReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<SmartIDReaderCardAdapter> SmartIDReaderUnit::getDefaultSmartIDReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<SmartIDReaderCardAdapter>(adapter);
	}

	std::string SmartIDReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool SmartIDReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	bool SmartIDReaderUnit::connectToReader()
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

	void SmartIDReaderUnit::disconnectFromReader()
	{
		getDataTransport()->disconnect();
	}

	std::vector<unsigned char> SmartIDReaderUnit::getPingCommand() const
	{
		std::vector<unsigned char> cmd;

		cmd.push_back(static_cast<unsigned char>(0x4f));

		return cmd;
	}

	std::string SmartIDReaderUnit::getInformation()
	{
		return BufferHelper::getStdString(getDefaultSmartIDReaderCardAdapter()->sendCommand(0x4F, std::vector<unsigned char>()));
	}

	std::string SmartIDReaderUnit::getFirmwareVersion()
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(0x00));

		return BufferHelper::getStdString(getDefaultSmartIDReaderCardAdapter()->sendCommand(0x4F, data));
	}

	void SmartIDReaderUnit::reboot()
	{
		getDefaultSmartIDReaderCardAdapter()->sendCommand(0xE0, std::vector<unsigned char>());
	}

	void SmartIDReaderUnit::resetRF(int offtime)
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(0x00));
		data.push_back(static_cast<unsigned char>(offtime));

		getDefaultSmartIDReaderCardAdapter()->sendCommand(0x4E, data, 1000 + offtime);
	}

	void SmartIDReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		ReaderUnit::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SmartIDReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		ReaderUnit::unSerialize(node);
	}

	boost::shared_ptr<SmartIDReaderProvider> SmartIDReaderUnit::getSmartIDReaderProvider() const
	{
		return boost::dynamic_pointer_cast<SmartIDReaderProvider>(getReaderProvider());
	}
}
