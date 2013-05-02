/**
 * \file PromagReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Promag reader unit.
 */

#include "PromagReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "PromagReaderProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/Chip.h"
#include "ReaderCardAdapters/PromagReaderCardAdapter.h"
#include <boost/filesystem.hpp>

namespace LOGICALACCESS
{
	PromagReaderUnit::PromagReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new PromagReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new PromagReaderCardAdapter());
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/PromagReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	PromagReaderUnit::~PromagReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> PromagReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void PromagReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string PromagReaderUnit::getName() const
	{
		string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string PromagReaderUnit::getConnectedName()
	{
		string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void PromagReaderUnit::setCardType(std::string cardType)
	{
		d_card_type = cardType;
	}

	bool PromagReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool inserted = false;
		unsigned int currentWait = 0;

		if(d_promagIdentifier.size() == 0)
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

	bool PromagReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if(d_promagIdentifier.size() == 0)
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

	bool PromagReaderUnit::connect()
	{
		return true;
	}

	void PromagReaderUnit::disconnect()
	{
	}

	boost::shared_ptr<Chip> PromagReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;
		std::vector<unsigned char> cmd;
		cmd.push_back(static_cast<unsigned char>('R'));

		std::vector<unsigned char> rawidbuf = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);
		if (rawidbuf.size() > 0)
		{
			string rawidstr = BufferHelper::getStdString(rawidbuf);
			std::vector<unsigned char> insertedId = XmlSerializable::formatHexString(rawidstr);	
			chip = ReaderUnit::createChip(
				(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
				insertedId
			);
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> PromagReaderUnit::createChip(std::string type)
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

	boost::shared_ptr<Chip> PromagReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > PromagReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<PromagReaderCardAdapter> PromagReaderUnit::getDefaultPromagReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<PromagReaderCardAdapter>(adapter);
	}

	string PromagReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool PromagReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool PromagReaderUnit::connectToReader()
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

	void PromagReaderUnit::disconnectFromReader()
	{
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void PromagReaderUnit::startAutoDetect()
	{
		if (d_port && d_port->getSerialPort()->deviceName() == "")
		{
			if (!LOGICALACCESS::Settings::getInstance().IsAutoDetectEnabled)
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

						boost::shared_ptr<PromagReaderUnit> testingReaderUnit(new PromagReaderUnit(*i));
						boost::shared_ptr<PromagReaderCardAdapter> testingCardAdapter(new PromagReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						
						std::vector<unsigned char> cmd;
						cmd.push_back(static_cast<unsigned char>('N'));

						std::vector<unsigned char> r = testingCardAdapter->sendCommand(cmd, LOGICALACCESS::Settings::getInstance().AutoDetectionTimeout);

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

	void PromagReaderUnit::configure()
	{
		configure(getSerialPort(), LOGICALACCESS::Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void PromagReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLOGICALACCESSException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLOGICALACCESSException, "Serial port name is empty ! Auto-detect failed !");

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

	void PromagReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void PromagReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<PromagReaderProvider> PromagReaderUnit::getPromagReaderProvider() const
	{
		return boost::dynamic_pointer_cast<PromagReaderProvider>(getReaderProvider());
	}

	bool PromagReaderUnit::retrieveReaderIdentifier()
	{
		bool ret;
		std::vector<unsigned char> cmd;
		try
		{
			cmd.push_back(static_cast<unsigned char>('N'));

			std::vector<unsigned char> r = getDefaultPromagReaderCardAdapter()->sendCommand(cmd);			

			d_promagIdentifier = r;
			ret = true;
		}
		catch(std::exception&)
		{
			ret = false;
		}

		return ret;
	}

	std::vector<unsigned char> PromagReaderUnit::getPromagIdentifier()
	{
		return d_promagIdentifier;
	}
}
