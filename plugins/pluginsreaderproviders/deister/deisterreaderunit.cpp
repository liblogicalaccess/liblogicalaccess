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

namespace logicalaccess
{
	DeisterReaderUnit::DeisterReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new DeisterReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<DeisterReaderCardAdapter> (new DeisterReaderCardAdapter()));
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/DeisterReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	DeisterReaderUnit::~DeisterReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> DeisterReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void DeisterReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string DeisterReaderUnit::getName() const
	{
		string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string DeisterReaderUnit::getConnectedName()
	{
		string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void DeisterReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool DeisterReaderUnit::waitInsertion(unsigned int maxwait)
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

	bool DeisterReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

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

	bool DeisterReaderUnit::connect()
	{
		return true;
	}

	void DeisterReaderUnit::disconnect()
	{

	}

	bool DeisterReaderUnit::connectToReader()
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

	void DeisterReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void DeisterReaderUnit::startAutoDetect()
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

						boost::shared_ptr<DeisterReaderUnit> testingReaderUnit(new DeisterReaderUnit(*i));
						boost::shared_ptr<DeisterReaderCardAdapter> testingCardAdapter(new DeisterReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						
						std::vector<unsigned char> cmd;
						cmd.push_back(static_cast<unsigned char>(0x0B));

						std::vector<unsigned char> pollBuf = testingCardAdapter->sendCommand(cmd, Settings::getInstance().AutoDetectionTimeout);

						INFO_SIMPLE_("Reader found ! Using this COM port !");
						d_port = (*i);
						found = true;
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

	boost::shared_ptr<Chip> DeisterReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>(0x0B));

		std::vector<unsigned char> pollBuf = getDefaultDeisterReaderCardAdapter()->sendCommand(cmd);
		if (pollBuf.size() > 0)
		{
			std::vector<unsigned char> tmpId;
			getDefaultDeisterReaderCardAdapter()->receiveCommand(pollBuf);
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
		switch(deisterCardType)
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

	boost::shared_ptr<Chip> DeisterReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;

			if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
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

	boost::shared_ptr<Chip> DeisterReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > DeisterReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<DeisterReaderCardAdapter> DeisterReaderUnit::getDefaultDeisterReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<DeisterReaderCardAdapter>(adapter);
	}

	string DeisterReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool DeisterReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	void DeisterReaderUnit::configure()
	{
		configure(getSerialPort(), Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void DeisterReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		try
		{
#ifndef _WINDOWS
			struct termios options = port->getSerialPort()->configuration();

			/* Set speed */
			cfsetispeed(&options, B9600);
			cfsetospeed(&options, B9600);

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
			options.BaudRate = CBR_9600;
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
#if !defined(__unix__)
				Sleep(Settings::getInstance().ConfigurationRetryTimeout);
#else
				sleep(Settings::getInstance().ConfigurationRetryTimeout);
#endif
				port->getSerialPort()->reopen();
				configure(getSerialPort(), false);
			}
		}
	}

	void DeisterReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void DeisterReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<DeisterReaderProvider> DeisterReaderUnit::getDeisterReaderProvider() const
	{
		return boost::dynamic_pointer_cast<DeisterReaderProvider>(getReaderProvider());
	}
}
