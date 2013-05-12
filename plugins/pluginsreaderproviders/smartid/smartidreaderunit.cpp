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
#include "mifarecardprovider.hpp"

namespace logicalaccess
{
	SmartIDReaderUnit::SmartIDReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new SmartIDReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new SmartIDReaderCardAdapter());
		d_ledBuzzerDisplay.reset(new SmartIDLEDBuzzerDisplay());
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/SmartIDReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	SmartIDReaderUnit::~SmartIDReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> SmartIDReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void SmartIDReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string SmartIDReaderUnit::getName() const
	{
		string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string SmartIDReaderUnit::getConnectedName()
	{
		string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void SmartIDReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
		d_readerCommunication = getReaderCommunication(cardType);
		boost::dynamic_pointer_cast<ReaderCardAdapter>(d_readerCommunication)->setReaderUnit(shared_from_this());
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
#elif defined(LINUX)
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
#elif defined(LINUX)
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
			boost::shared_ptr<CardProvider> cp;
			boost::shared_ptr<Commands> commands;

			if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				rca.reset(new MifareSmartIDReaderCardAdapter());
				cp.reset(new MifareCardProvider());
				commands.reset(new MifareSmartIDCommands());
			}
			else if (type == "DESFireEV1")
			{
				rca = getDefaultReaderCardAdapter();
				cp = LibraryManager::getInstance()->getCardProvider("DESFireEV1");
				commands = LibraryManager::getInstance()->getCommands("DESFireEV1ISO7816");
				*(void**)(&setcryptocontextfct) = LibraryManager::getInstance()->getFctFromName("setCryptoContextDESFireEV1ISO7816Commands", LibraryManager::READERS_TYPE);
				setcryptocontextfct(&commands, &chip);
			}
			else if (type == "DESFire")
			{
				rca = getDefaultReaderCardAdapter();
				cp = LibraryManager::getInstance()->getCardProvider("DESFire");
				commands = LibraryManager::getInstance()->getCommands("DESFireISO7816");
				*(void**)(&setcryptocontextfct) = LibraryManager::getInstance()->getFctFromName("setCryptoContextDESFireEV1ISO7816Commands", LibraryManager::READERS_TYPE);
				setcryptocontextfct(&commands, &chip);
			}
			else if (type == "Twic")
			{
				rca = getDefaultReaderCardAdapter();
				cp = LibraryManager::getInstance()->getCardProvider("Twic");
				commands = LibraryManager::getInstance()->getCommands("TwicISO7816");
			}
			else
				return chip;

			rca->setReaderUnit(shared_from_this());
			if (commands)
			{
				commands->setReaderCardAdapter(rca);
			}
			if (cp)
			{
				cp->setCommands(commands);
			}
			chip->setCardProvider(cp);
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
		return (d_insertedChip);
	}

	bool SmartIDReaderUnit::connectToReader()
	{
		bool ret = false;

		startAutoDetect();

		EXCEPTION_ASSERT_WITH_LOG(getSerialPort(), LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(getSerialPort()->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		if (!getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->open();
			configure();
			ret = true;
		}
		else
		{
			ret = true;
		}

		return ret;
	}

	void SmartIDReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void SmartIDReaderUnit::startAutoDetect()
	{
		if (d_port && d_port->getSerialPort()->deviceName() == "")
		{
			if (!Settings::getInstance().IsAutoDetectEnabled)
			{
				INFO_SIMPLE_("Auto detection is disabled through settings !");
				return;
			}

			INFO_SIMPLE_("Serial port is empty ! Starting Auto COM Port Detection...");
			std::vector<boost::shared_ptr<SerialPortXml> > ports;
			if (SerialPortXml::EnumerateUsingCreateFile(ports) && !ports.empty())
			{
				bool found = false;
				for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i  = ports.begin(); i != ports.end() && !found; ++i)
				{
					try
					{
						INFO_("Processing port {%s}...", (*i)->getSerialPort()->deviceName().c_str());
						(*i)->getSerialPort()->open();
						configure((*i), false);

						boost::shared_ptr<SmartIDReaderUnit> testingReaderUnit(new SmartIDReaderUnit(*i));
						boost::shared_ptr<SmartIDReaderCardAdapter> testingCardAdapter(new SmartIDReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);

						std::vector<unsigned char> r = testingCardAdapter->sendCommand(0x4F, std::vector<unsigned char>(), Settings::getInstance().AutoDetectionTimeout);

						if (r.size() > 0)
						{
							INFO_SIMPLE_("Reader found ! Using this COM port !");
							d_port = (*i);
							found = true;
						}
					}
					catch (std::exception& e)
					{
						ERROR_("Exception {%s}", e.what());
					}
					catch (...)
					{
						ERROR_SIMPLE_("Exception received !");
					}

					if ((*i)->getSerialPort()->isOpen())
					{
						(*i)->getSerialPort()->close();
					}
				}

				if (!found)
				{
					INFO_SIMPLE_("NO Reader found on COM port...");
				}
				else
				{
					d_isAutoDetected = true;
				}
			}
			else
			{
				WARNING_SIMPLE_("No COM Port detected !");
			}
		}
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

	void SmartIDReaderUnit::configure()
	{
		configure(getSerialPort(), Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void SmartIDReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		try
		{
#ifndef _WINDOWS
			struct termios options = port->getSerialPort()->configuration();

			/* Set speed */
			cfsetispeed(&options, getSmartIDConfiguration()->getSystemBaudrate());
			cfsetospeed(&options, getSmartIDConfiguration()->getSystemBaudrate());

			/* Enable the receiver and set local mode */
			options.c_cflag |= (CLOCAL | CREAD);

			/* Set character size and parity check */
			/* 8N1 */
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;

			/* Disable parity check and fancy stuff */
			options.c_iflag &= ~ICRNL;
			options.c_iflag &= ~INPCK;
			options.c_iflag &= ~ISTRIP;

			/* Disable software flow control */
			options.c_iflag &= ~(IXON | IXOFF | IXANY);

			/* RAW input */
			options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

			/* RAW output */
			options.c_oflag &= ~OPOST;

			/* Timeouts */
			options.c_cc[VMIN] = 1;
			options.c_cc[VTIME] = 5;

			port->getSerialPort()->setConfiguration(options);
#else
			DCB options = port->getSerialPort()->configuration();
			options.BaudRate = getSmartIDConfiguration()->getSystemBaudrate();
			options.fBinary = TRUE;               // Binary mode; no EOF check
			options.fParity = FALSE;               // Enable parity checking
			options.fOutxCtsFlow = FALSE;         // No CTS output flow control
			options.fOutxDsrFlow = FALSE;         // No DSR output flow control
			options.fDtrControl = DTR_CONTROL_DISABLE;
													// DTR flow control type
			options.fDsrSensitivity = FALSE;      // DSR sensitivity
			options.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
			options.fOutX = FALSE;                // No XON/XOFF out flow control
			options.fInX = FALSE;                 // No XON/XOFF in flow control
			options.fErrorChar = FALSE;           // Disable error replacement
			options.fNull = FALSE;                // Disable null stripping
			options.fRtsControl = RTS_CONTROL_DISABLE;
													// RTS flow control
			options.fAbortOnError = FALSE;        // Do not abort reads/writes on
													// error
			options.ByteSize = 8;                 // Number of bits/byte, 4-8
			options.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
			options.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2
			port->getSerialPort()->setConfiguration(options);
#endif
		}
		catch(std::exception& e)
		{
			if (retryConfiguring)
			{
				// Strange stuff is going here... by waiting and reopening the COM port (maybe for system cleanup), it's working !
				std::string portn = port->getSerialPort()->deviceName();
				WARNING_("Exception received {%s} ! Sleeping {%d} milliseconds -> Reopen serial port {%s} -> Finally retry  to configure...",
							e.what(), Settings::getInstance().ConfigurationRetryTimeout, portn.c_str());
#ifndef __linux__
				Sleep(Settings::getInstance().ConfigurationRetryTimeout);
#else
				sleep(Settings::getInstance().ConfigurationRetryTimeout);
#endif
				port->getSerialPort()->reopen();
				configure(getSerialPort(), false);
			}
		}
	}

	void SmartIDReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SmartIDReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<SmartIDReaderProvider> SmartIDReaderUnit::getSmartIDReaderProvider() const
	{
		return boost::dynamic_pointer_cast<SmartIDReaderProvider>(getReaderProvider());
	}
}
