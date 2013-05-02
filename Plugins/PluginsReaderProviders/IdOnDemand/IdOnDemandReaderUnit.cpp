/**
 * \file IdOnDemandReaderUnit.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief IdOnDemand reader unit.
 */

#include "IdOnDemandReaderUnit.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "IdOnDemandReaderProvider.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"
#include "logicalaccess/Cards/Chip.h"
#include "ReaderCardAdapters/IdOnDemandReaderCardAdapter.h"
#include "Commands/GenericTagIdOnDemandCardProvider.h"
#include "Commands/GenericTagIdOnDemandAccessControlCardService.h"
#include <boost/filesystem.hpp>

namespace LOGICALACCESS
{
	IdOnDemandReaderUnit::IdOnDemandReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new IdOnDemandReaderUnitConfiguration());
		d_defaultReaderCardAdapter.reset(new IdOnDemandReaderCardAdapter());
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/IdOnDemandReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	IdOnDemandReaderUnit::~IdOnDemandReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> IdOnDemandReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void IdOnDemandReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string IdOnDemandReaderUnit::getName() const
	{
		string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string IdOnDemandReaderUnit::getConnectedName()
	{
		string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void IdOnDemandReaderUnit::setCardType(std::string cardType)
	{
		INFO_("Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	void IdOnDemandReaderUnit::authenticateSDK()
	{
		authenticateSDK(getIdOnDemandConfiguration()->getAuthCode());
	}

	void IdOnDemandReaderUnit::authenticateSDK(std::string authCode)
	{
		char cmd[64];
#ifndef __linux__
		sprintf_s(cmd, sizeof(cmd), "AUTH %s", authCode.c_str());
#else
		sprintf(cmd, "AUTH %s", authCode.c_str());
#endif
		std::string strcmd = std::string(cmd);
		getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
	}

	void IdOnDemandReaderUnit::beep()
	{
		std::string strcmd = std::string("BEEP");
		getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
	}

	bool IdOnDemandReaderUnit::read()
	{
		bool ret = true;
		try
		{
			std::string strcmd = std::string("READ");
			getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
		}
		catch(CardException& e)
		{
			ERROR_("Exception Read {%s}", e.what());
			ret = false;
		}

		return ret;
	}

	bool IdOnDemandReaderUnit::verify()
	{
		bool ret = true;
		try
		{
			std::string strcmd = std::string("VERIFY");
			getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
		}
		catch(CardException& e)
		{
			ERROR_("Exception Verify {%s}", e.what());
			ret = false;
		}

		return ret;
	}

	bool IdOnDemandReaderUnit::write()
	{
		bool ret = true;
		try
		{
			std::string strcmd = std::string("WRITE");
			getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()), 4000);
		}
		catch(CardException& e)
		{
			ERROR_("Exception Write {%s}", e.what());
			ret = false;
		}

		return ret;
	}

	void IdOnDemandReaderUnit::activateEMCard(bool activate)
	{
		std::string strcmd = "EMCARD ";
		if (activate)
		{
			strcmd += "ON";
		}
		else
		{
			strcmd += "FALSE";
		}
		getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
	}

	void IdOnDemandReaderUnit::activateAtmelCard(bool activate)
	{
		std::string strcmd = "ATCARD ";
		if (activate)
		{
			strcmd += "ON";
		}
		else
		{
			strcmd += "FALSE";
		}
		getDefaultIdOnDemandReaderCardAdapter()->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
	}

	bool IdOnDemandReaderUnit::waitInsertion(unsigned int maxwait)
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

	bool IdOnDemandReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool removed = false;
		unsigned int currentWait = 0;

		if (d_insertedChip)
		{
			do
			{
				boost::shared_ptr<Chip> chip = getChipInAir();
				if (!chip)
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

	bool IdOnDemandReaderUnit::connect()
	{
		return (d_insertedChip);
	}

	void IdOnDemandReaderUnit::disconnect()
	{
	}

	boost::shared_ptr<Chip> IdOnDemandReaderUnit::getChipInAir()
	{
		boost::shared_ptr<Chip> chip;

		// Change the reader state, but no other function to achieve the card detection...
		if (read())
		{
			chip = createChip("GenericTag");
		}

		return chip;
	}
	
	boost::shared_ptr<Chip> IdOnDemandReaderUnit::createChip(std::string type)
	{
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;

			if (type == "GenericTag")
			{
				rca = getDefaultReaderCardAdapter();
				cp.reset(new GenericTagIdOnDemandCardProvider());
			}
			else
					return chip;

			rca->setReaderUnit(shared_from_this());
			if (cp)
			{
				chip->setCardProvider(cp);
				boost::dynamic_pointer_cast<GenericTagIdOnDemandCardProvider>(cp)->setReaderCardAdapter(rca);
			}
			
		}
		return chip;
	}

	boost::shared_ptr<Chip> IdOnDemandReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > IdOnDemandReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<IdOnDemandReaderCardAdapter> IdOnDemandReaderUnit::getDefaultIdOnDemandReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<IdOnDemandReaderCardAdapter>(adapter);
	}

	string IdOnDemandReaderUnit::getReaderSerialNumber()
	{
		string ret;

		return ret;
	}

	bool IdOnDemandReaderUnit::isConnected()
	{
		return (d_insertedChip);
	}

	bool IdOnDemandReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Connecting to reader...");
		bool ret = false;

		startAutoDetect();

		EXCEPTION_ASSERT_WITH_LOG(getSerialPort(), LibLOGICALACCESSException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(getSerialPort()->getSerialPort()->deviceName() != "", LibLOGICALACCESSException, "Serial port name is empty ! Auto-detect failed !");

		if (!getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->open();
			configure();

			authenticateSDK();

			ret = true;
		}
		else
		{
			ret = true;
		}

		return ret;
	}

	void IdOnDemandReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Disconnecting from reader...");
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			// Close the session with the reader
			//authenticateSDK(0000);	 // A wrong authenticate code logout the session

			getSerialPort()->getSerialPort()->close();
		}
	}

	void IdOnDemandReaderUnit::startAutoDetect()
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

						boost::shared_ptr<IdOnDemandReaderUnit> testingReaderUnit(new IdOnDemandReaderUnit(*i));

						boost::shared_ptr<IdOnDemandReaderCardAdapter> testingCardAdapter(new IdOnDemandReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						std::string strcmd = "BEEP";
						testingCardAdapter->sendCommand(std::vector<unsigned char>(strcmd.begin(), strcmd.end()));
						
						// If no exception, the reader has been detected !
						INFO_SIMPLE_("Reader found ! Using this COM port !");
						d_port = (*i);
						found = true;
					}
					catch (CardException&)
					{
						INFO_SIMPLE_("Error returned - So serial is good !");
						d_port = (*i);
						found = true;
					}
					catch (std::invalid_argument& e)
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

	void IdOnDemandReaderUnit::configure()
	{
		configure(getSerialPort(), LOGICALACCESS::Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void IdOnDemandReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		INFO_SIMPLE_("Configuring serial port...");
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
			options.fTXContinueOnXoff = FALSE;     // XOFF continues Tx
			options.fOutX = FALSE;                // No XON/XOFF out flow control
			options.fInX = FALSE;                 // No XON/XOFF in flow control
			options.fErrorChar = FALSE;           // Disable error replacement
			options.fNull = FALSE;                // Disable null stripping
			options.fRtsControl = RTS_CONTROL_DISABLE;

			options.XoffLim = FALSE;
			options.XonLim = FALSE;
			options.XonChar = 0x00;
			options.XoffChar = 0x00;
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
			WARNING_("Exception received {%s} !", e.what());
			if (retryConfiguring)
			{
				// Strange stuff is going here... by waiting and reopening the COM port (maybe for system cleanup), it's working !
				std::string portn = port->getSerialPort()->deviceName();
				WARNING_("Sleeping {%d} milliseconds -> Reopen serial port {%s} -> Finally retry  to configure...",
							e.what(), LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout, portn.c_str());
#ifndef __linux__
				Sleep(LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout);
#else
				sleep(LOGICALACCESS::Settings::getInstance().ConfigurationRetryTimeout);
#endif
				port->getSerialPort()->reopen();
				configure(port, false);
			}
		}
	}

	void IdOnDemandReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void IdOnDemandReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<IdOnDemandReaderProvider> IdOnDemandReaderUnit::getIdOnDemandReaderProvider() const
	{
		return boost::dynamic_pointer_cast<IdOnDemandReaderProvider>(getReaderProvider());
	}
}
