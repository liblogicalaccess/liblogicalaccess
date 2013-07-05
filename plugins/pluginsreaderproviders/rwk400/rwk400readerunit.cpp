/**
 * \file rwk400readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 reader unit.
 */

#include "rwk400readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "logicalaccess/bufferhelper.hpp"
#include "rwk400readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/rwk400readercardadapter.hpp"
#include "commands/desfirerwk400commands.hpp"
#include "desfirecardprovider.hpp"
#include "desfireev1cardprovider.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include <boost/filesystem.hpp>

namespace logicalaccess
{

	Rwk400ReaderUnit::Rwk400ReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new Rwk400ReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<Rwk400ReaderCardAdapter> (new Rwk400ReaderCardAdapter()));
		//d_ledBuzzerDisplay.reset(new Rwk400LEDBuzzerDisplay());
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/Rwk400ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	Rwk400ReaderUnit::~Rwk400ReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> Rwk400ReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void Rwk400ReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string Rwk400ReaderUnit::getName() const
	{
		string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string Rwk400ReaderUnit::getConnectedName()
	{
		string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void Rwk400ReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
		d_readerCommunication = getReaderCommunication(cardType);
		boost::dynamic_pointer_cast<ReaderCardAdapter>(d_readerCommunication)->setReaderUnit(shared_from_this());
	}

	boost::shared_ptr<ReaderCommunication> Rwk400ReaderUnit::getReaderCommunication(std::string cardType)
	{
		boost::shared_ptr<ReaderCommunication> ret;

		ret.reset(new Rwk400ReaderCardAdapter());
		return ret;
	}

	bool Rwk400ReaderUnit::waitInsertion(unsigned int maxwait)
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

	bool Rwk400ReaderUnit::waitRemoval(unsigned int maxwait)
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

	bool Rwk400ReaderUnit::connect()
	{
		COM_SIMPLE_("connect to the chip");
		bool ret = true;
		return ret;
	}

	void Rwk400ReaderUnit::disconnect()
	{
	}

	std::vector<unsigned char> Rwk400ReaderUnit::getCardSerialNumber()
	{
		std::vector<unsigned char> csn = selectCard ();
		if (csn.size () > 0)
			csn.erase (csn.begin());
		return csn;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;
			boost::shared_ptr<Commands> commands;

			if (type == "DESFire")
			{
				rca = getDefaultRwk400ReaderCardAdapter();
				cp.reset(new DESFireCardProvider());
				commands.reset(new DesfireRwk400Commands());
				boost::dynamic_pointer_cast<DesfireRwk400Commands>(commands)->getCrypto().setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(chip->getProfile()), chip->getChipIdentifier());
			}
			else if (type == "GenericTag")
			{
				rca = getDefaultRwk400ReaderCardAdapter();
			}
			else
				return chip;

			if (rca)
			{
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
		}
		return chip;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > Rwk400ReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<Rwk400ReaderCardAdapter> Rwk400ReaderUnit::getDefaultRwk400ReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<Rwk400ReaderCardAdapter>(adapter);
	}

	bool Rwk400ReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool Rwk400ReaderUnit::connectToReader()
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

	void Rwk400ReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void Rwk400ReaderUnit::startAutoDetect()
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

						boost::shared_ptr<Rwk400ReaderUnit> testingReaderUnit(new Rwk400ReaderUnit(*i));
						boost::shared_ptr<Rwk400ReaderCardAdapter> testingCardAdapter(new Rwk400ReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						std::vector<unsigned char> testcommand;
						testcommand.push_back(0x80);							// CLA
						testcommand.push_back(RWK400Commands::READ_EEPROM);		// INS
						testcommand.push_back(0x00);							// P1
						testcommand.push_back(0x00);							// P2
						testcommand.push_back(0x01);							// P3
						std::vector<unsigned char> r = testingCardAdapter->sendCommand(testcommand, Settings::getInstance().AutoDetectionTimeout);
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

	std::string Rwk400ReaderUnit::getReaderSerialNumber()
	{
		return "";
	}

	void Rwk400ReaderUnit::configure()
	{
		configure(getSerialPort(), Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void Rwk400ReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
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
			options.BaudRate = getRwk400Configuration()->getSystemBaudrate();
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

	void Rwk400ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void Rwk400ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<Rwk400ReaderProvider> Rwk400ReaderUnit::getRwk400ReaderProvider() const
	{
		return boost::dynamic_pointer_cast<Rwk400ReaderProvider>(getReaderProvider());
	}

	void Rwk400ReaderUnit::resetRF(int /*offtime*/)
	{
		std::vector<unsigned char> command;
		command.push_back(0x80);							// CLA
		command.push_back(RWK400Commands::RESET_RF);		// INS
		command.push_back(0x40);							// P1
		command.push_back(0x00);							// P2
		command.push_back(0x00);							// P3
		try
		{
			getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{

		}
	}

	std::vector<unsigned char> Rwk400ReaderUnit::selectCard()
	{
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back(0x80);							// CLA
		command.push_back(RWK400Commands::SELECT_CARD);		// INS
		command.push_back(0x00);							// P1
		command.push_back(0x0A);							// P2
		command.push_back(0x09);							// P3
		try
		{
			answer = getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{

		}
		return answer;
	}

	boost::shared_ptr<Chip> Rwk400ReaderUnit::getChipInAir(unsigned int maxwait)
	{
		INFO_SIMPLE_("Starting get chip in air...");

		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> buf;
		unsigned int currentWait = 0;
		while (!chip && (maxwait == 0 || currentWait < maxwait))
		{
			buf = selectCard ();
			COM_("Select Card : %s",BufferHelper::getHex (buf).c_str());
			if (buf.size () > 1)
			{
				if (buf[0] == RWK400_MAFARE_DESFIRE_CHIP)
				{
					if (rats ())
					{
						chip = createChip ("DESFire");
						buf.erase (buf.begin());
						chip->setChipIdentifier(buf);
					}
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
		return chip;
	}

	bool Rwk400ReaderUnit::rats()
	{
		bool res = true;
		std::vector<unsigned char> command;
		std::vector<unsigned char> answer;
		command.push_back (0x80);						// CLA
		command.push_back (RWK400Commands::TRANSMIT);	// INS
		command.push_back (0xF7);						// P1
		command.push_back (0x06);						// P2
		command.push_back (0x02);						// P3
		command.push_back (0xE0);						// RATS
		command.push_back (0x80);						// P1
		try
		{
			answer = getDefaultRwk400ReaderCardAdapter()->sendCommand(command);
		}
		catch(std::exception&)
		{
			res = false;
		}
		return res;
	}
}
