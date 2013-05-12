/**
 * \file AxessTMCLegicReaderUnit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic reader unit.
 */

#include "AxessTMCLegicReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "AxessTMCLegicReaderProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/Chip.h"
#include "ReaderCardAdapters/AxessTMCLegicReaderCardAdapter.h"
#include <boost/filesystem.hpp>

namespace logicalaccess
{
	AxessTMCLegicReaderUnit::AxessTMCLegicReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{		
		d_readerUnitConfig.reset(new AxessTMCLegicReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new AxessTMCLegicReaderCardAdapter());
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/AxessTMCLegicReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	AxessTMCLegicReaderUnit::~AxessTMCLegicReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> AxessTMCLegicReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void AxessTMCLegicReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string AxessTMCLegicReaderUnit::getName() const
	{
		std::string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string AxessTMCLegicReaderUnit::getConnectedName()
	{
		std::string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}	

	void AxessTMCLegicReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool AxessTMCLegicReaderUnit::waitInsertion(unsigned int maxwait)
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
				Sleep(250);
#elif defined(LINUX)
				usleep(250000);
#endif
				currentWait += 250;
			}
		} while (!inserted && (maxwait == 0 || currentWait < maxwait));

		return inserted;
	}

	bool AxessTMCLegicReaderUnit::waitRemoval(unsigned int maxwait)
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
					Sleep(250);
	#elif defined(LINUX)
					usleep(250000);
	#endif
					currentWait += 250;
				}
			} while (!removed && (maxwait == 0 || currentWait < maxwait));
		}

		return removed;
	}

	bool AxessTMCLegicReaderUnit::connect()
	{
		return true;
	}

	void AxessTMCLegicReaderUnit::disconnect()
	{

	}

	bool AxessTMCLegicReaderUnit::connectToReader()
	{
		bool ret = false;

		startAutoDetect();

		EXCEPTION_ASSERT_WITH_LOG(getSerialPort(), LibLOGICALACCESSException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(getSerialPort()->getSerialPort()->deviceName() != "", LibLOGICALACCESSException, "Serial port name is empty ! Auto-detect failed !");

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

	void AxessTMCLegicReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void AxessTMCLegicReaderUnit::startAutoDetect()
	{
#ifdef _WINDOWS
		if (d_port && d_port->getSerialPort()->deviceName() == "")
		{
			if (!LOGICALACCESS::Settings::getInstance().IsAutoDetectEnabled)
			{
				INFO_("Auto detection is disabled through settings !");
				return;
			}

			INFO_("Serial port is empty ! Starting Auto COM Port Detection...");
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

						boost::shared_ptr<AxessTMCLegicReaderUnit> testingReaderUnit(new AxessTMCLegicReaderUnit(*i));
						boost::shared_ptr<AxessTMCLegicReaderCardAdapter> testingCardAdapter(new AxessTMCLegicReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						
						std::vector<unsigned char> cmd;
						cmd.push_back(0xb0);
						cmd.push_back(0xb2);

						std::vector<unsigned char> r = testingCardAdapter->sendCommand(cmd, LOGICALACCESS::Settings::getInstance().AutoDetectionTimeout);

						INFO_("Reader found ! Using this COM port !");
						d_port = (*i);
						found = true;
					}
					catch (std::exception& e)
					{
						ERROR_("Exception {%s}", e.what());
					}
					catch (...)
					{
						ERROR_("Exception received !");
					}

					if ((*i)->getSerialPort()->isOpen())
					{
						(*i)->getSerialPort()->close();
					}
				}

				if (!found)
				{
					INFO_("NO Reader found on COM port...");
				}
				else
				{
					d_isAutoDetected = true;
				}
			}
			else
			{
				WARNING_("No COM Port detected !");
			}
		}
#endif
	}

	void AxessTMCLegicReaderUnit::idleState()
	{
		std::vector<unsigned char> cmd;
		cmd.push_back(0x14);
		cmd.push_back(0x01);
		cmd.push_back(0x16);

		getDefaultAxessTMCLegicReaderCardAdapter()->sendCommand(cmd);
	}

	boost::shared_ptr<Chip> AxessTMCLegicReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;

#ifdef _WINDOWS
		std::vector<unsigned char> cmd;
		cmd.push_back(0xb0);
		cmd.push_back(0xb2);

		std::vector<unsigned char> response = getDefaultAxessTMCLegicReaderCardAdapter()->sendCommand(cmd);
		if (response.size() > 4)
		{
			unsigned char idlength = response[4];
			EXCEPTION_ASSERT_WITH_LOG((response.size() - 5) >= idlength, LibLOGICALACCESSException, "Bad id buffer length.");
			
			chip = ReaderUnit::createChip(
				(d_card_type == "UNKNOWN" ? "LegicPrime" : d_card_type),
				std::vector<unsigned char>(response.begin() + 5, response.begin() + 5 + idlength)
			);
		}
#endif

		return chip;
	}

	boost::shared_ptr<Chip> AxessTMCLegicReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;

		if (type == "LegicPrime")
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

	boost::shared_ptr<Chip> AxessTMCLegicReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > AxessTMCLegicReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<AxessTMCLegicReaderCardAdapter> AxessTMCLegicReaderUnit::getDefaultAxessTMCLegicReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<AxessTMCLegicReaderCardAdapter>(adapter);
	}

	string AxessTMCLegicReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool AxessTMCLegicReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}	

	void AxessTMCLegicReaderUnit::configure()
	{
		configure(getSerialPort(), LOGICALACCESS::Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void AxessTMCLegicReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLOGICALACCESSException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLOGICALACCESSException, "Serial port name is empty ! Auto-detect failed !");

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
							e.what(), LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout, portn.c_str());
#ifndef __linux__
				Sleep(LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout);
#else
				sleep(LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout);
#endif
				port->getSerialPort()->reopen();
				configure(getSerialPort(), false);
			}
		}
	}

	void AxessTMCLegicReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AxessTMCLegicReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<AxessTMCLegicReaderProvider> AxessTMCLegicReaderUnit::getAxessTMCLegicReaderProvider() const
	{
		return boost::dynamic_pointer_cast<AxessTMCLegicReaderProvider>(getReaderProvider());
	}
}
