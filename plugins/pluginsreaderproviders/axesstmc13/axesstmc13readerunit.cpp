/**
 * \file axesstmc13readerunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader unit.
 */

#include "axesstmc13readerunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "axesstmc13readerprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/readerproviders/serialport.hpp"
#include "readercardadapters/axesstmc13readercardadapter.hpp"
#include <boost/filesystem.hpp>

namespace logicalaccess
{
	AxessTMC13ReaderUnit::AxessTMC13ReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new AxessTMC13ReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<AxessTMC13ReaderCardAdapter> (new AxessTMC13ReaderCardAdapter()));
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/AxessTMC13ReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	AxessTMC13ReaderUnit::~AxessTMC13ReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> AxessTMC13ReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void AxessTMC13ReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string AxessTMC13ReaderUnit::getName() const
	{
		std::string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string AxessTMC13ReaderUnit::getConnectedName()
	{
		std::string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void AxessTMC13ReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool AxessTMC13ReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		if(d_tmcIdentifier.size() == 0)
		{
			retrieveReaderIdentifier();
		}

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

	bool AxessTMC13ReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if(d_tmcIdentifier.size() == 0)
		{
			retrieveReaderIdentifier();
		}

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

	bool AxessTMC13ReaderUnit::connect()
	{
		return true;
	}

	void AxessTMC13ReaderUnit::disconnect()
	{

	}

	bool AxessTMC13ReaderUnit::connectToReader()
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

	void AxessTMC13ReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void AxessTMC13ReaderUnit::startAutoDetect()
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

						boost::shared_ptr<AxessTMC13ReaderUnit> testingReaderUnit(new AxessTMC13ReaderUnit(*i));
						boost::shared_ptr<AxessTMC13ReaderCardAdapter> testingCardAdapter(new AxessTMC13ReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						
						std::vector<unsigned char> cmd;
						cmd.push_back(static_cast<unsigned char>('v'));

						std::vector<unsigned char> r = testingCardAdapter->sendCommand(cmd, Settings::getInstance().AutoDetectionTimeout);
						if (r.size() >= 3 && r[0] == 'T' && r[1] == 'M' && r[2] == 'C')
						{
							INFO_SIMPLE_("Reader found ! Using this COM port !");
							d_port = (*i);
							found = true;
						}
					}
					catch (LibLogicalAccessException& e)
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

	boost::shared_ptr<Chip> AxessTMC13ReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>('s'));

		std::vector<unsigned char> tmpASCIIId = getDefaultAxessTMC13ReaderCardAdapter()->sendCommand(cmd);
		if (tmpASCIIId.size() > 0 && tmpASCIIId[0] != 'N')
		{
			unsigned long long l = atoull(BufferHelper::getStdString(tmpASCIIId));
			char bufTmpId[64];
			sprintf(bufTmpId, "%llx", l);		
			chip = ReaderUnit::createChip(
				(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
				formatHexString(std::string(bufTmpId))
			);
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> AxessTMC13ReaderUnit::createChip(std::string type)
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

	boost::shared_ptr<Chip> AxessTMC13ReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > AxessTMC13ReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<AxessTMC13ReaderCardAdapter> AxessTMC13ReaderUnit::getDefaultAxessTMC13ReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<AxessTMC13ReaderCardAdapter>(adapter);
	}

	string AxessTMC13ReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool AxessTMC13ReaderUnit::isConnected()
	{
		return bool(d_insertedChip);
	}

	void AxessTMC13ReaderUnit::configure()
	{
		configure(getSerialPort(), Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void AxessTMC13ReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		try
		{
#ifndef _WINDOWS
			struct termios options = port->getSerialPort()->configuration();

			/* Set speed */
			cfsetispeed(&options, B57600);
			cfsetospeed(&options, B57600);

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
			options.BaudRate = CBR_57600;
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

	void AxessTMC13ReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AxessTMC13ReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<AxessTMC13ReaderProvider> AxessTMC13ReaderUnit::getAxessTMC13ReaderProvider() const
	{
		return boost::dynamic_pointer_cast<AxessTMC13ReaderProvider>(getReaderProvider());
	}

	bool AxessTMC13ReaderUnit::retrieveReaderIdentifier()
	{
		bool ret;
		std::vector<unsigned char> cmd;
		try
		{
			cmd.push_back(static_cast<unsigned char>('v'));

			std::vector<unsigned char> r = getDefaultAxessTMC13ReaderCardAdapter()->sendCommand(cmd);
			EXCEPTION_ASSERT_WITH_LOG(r.size() >= 3, std::invalid_argument, "Bad response getting TMC reader identifier.");
			EXCEPTION_ASSERT_WITH_LOG(r[0] == 'T', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 0 doesn't match.");
			EXCEPTION_ASSERT_WITH_LOG(r[1] == 'M', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 1 doesn't match.");
			EXCEPTION_ASSERT_WITH_LOG(r[2] == 'C', std::invalid_argument, "Bad response getting TMC reader identifier, identifier byte 2 doesn't match.");

			d_tmcIdentifier = r;
			ret = true;
		}
		catch(std::invalid_argument&)
		{
			ret = false;
		}

		return ret;
	}

	std::vector<unsigned char> AxessTMC13ReaderUnit::getTMCIdentifier()
	{
		return d_tmcIdentifier;
	}
}
