/**
 * \file admittoreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto reader unit.
 */

#include "admittoreaderunit.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "admittoreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "readercardadapters/admittoreadercardadapter.hpp"
#include <boost/filesystem.hpp>
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

namespace logicalaccess
{
	/*
	 *
	 *  WARNING: THE READER ONLY WORKS WITH X86 SYSTEMS !
	 *  Admitto reader only send ID through Serial port on card insertion and [0x4e] on card removed. It doesn't support any request command !  
	 *
	 */

	AdmittoReaderUnit::AdmittoReaderUnit(boost::shared_ptr<SerialPortXml> port)
		: ReaderUnit()
	{
		d_readerUnitConfig.reset(new AdmittoReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<AdmittoReaderCardAdapter> (new AdmittoReaderCardAdapter()));
		d_port = port;
		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/AdmittoReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		if (!d_port)
		{
			d_port.reset(new SerialPortXml(""));
		}
		d_isAutoDetected = false;
	}

	AdmittoReaderUnit::~AdmittoReaderUnit()
	{
		disconnectFromReader();
	}

	boost::shared_ptr<SerialPortXml> AdmittoReaderUnit::getSerialPort()
	{
		return d_port;
	}

	void AdmittoReaderUnit::setSerialPort(boost::shared_ptr<SerialPortXml> port)
	{
		if (port)
		{
			INFO_("Setting serial port {%s}...", port->getSerialPort()->deviceName().c_str());
			d_port = port;
		}
	}

	std::string AdmittoReaderUnit::getName() const
	{
		std::string ret;
		if (d_port && !d_isAutoDetected)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	std::string AdmittoReaderUnit::getConnectedName()
	{
		std::string ret;
		if (d_port)
		{
			ret = d_port->getSerialPort()->deviceName();
		}
		return ret;
	}

	void AdmittoReaderUnit::setCardType(std::string cardType)
	{
		INFO_("Setting card type {0x%s(%s)}", cardType.c_str(), cardType.c_str());
		d_card_type = cardType;
	}

	bool AdmittoReaderUnit::waitInsertion(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance().IsLogEnabled;
		if (oldValue && !Settings::getInstance().SeeWaitInsertionLog)
		{
			Settings::getInstance().IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		INFO_("Waiting insertion... max wait {%u}", maxwait);

		bool inserted = false;
		unsigned int currentWait = 0;
		
		try
		{
			// Admitto reader doesn't handle commands but we want to simulate the same behavior that for all readers
			// So we send a dummy commmand which does nothing
			do
			{
				try
				{
					std::vector<unsigned char> cmd;
					cmd.push_back(0xff);	// trick

					std::vector<unsigned char> buf = getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
					if (buf.size() > 0)
					{
						if (buf[0] == 0x4e)
						{
							INFO_SIMPLE_("[Card removed] response received when we were on WaitInsertion! We ignore this response and wait for valid card number.");
						}
						else
						{
							unsigned int l = static_cast<unsigned int>(atoull(BufferHelper::getStdString(buf)));
							char bufTmpId[64];
							sprintf(bufTmpId, "%08x", l);
							d_insertedChip = ReaderUnit::createChip(
								(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
								formatHexString(std::string(bufTmpId))
							);
							INFO_SIMPLE_("Chip detected !");
							inserted = true;
						}
					}
				}
				catch (std::exception&)
				{
					// No response received is ignored !
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
			} while (!inserted && (maxwait == 0 || currentWait < maxwait));
		}
		catch(...)
		{
			Settings::getInstance().IsLogEnabled = oldValue;
			throw;
		}

		INFO_("Returns card inserted ? {%d} function timeout expired ? {%d}", inserted, (maxwait != 0 && currentWait >= maxwait));
		Settings::getInstance().IsLogEnabled = oldValue;

		return inserted;
	}

	bool AdmittoReaderUnit::waitRemoval(unsigned int maxwait)
	{
		bool oldValue = Settings::getInstance().IsLogEnabled;
		if (oldValue && !Settings::getInstance().SeeWaitRemovalLog)
		{
			Settings::getInstance().IsLogEnabled = false;		// Disable logs for this part (otherwise too much log output in file)
		}

		INFO_("Waiting removal... max wait {%u}", maxwait);
		bool removed = false;
		unsigned int currentWait = 0;

		try
		{
			// The inserted chip will stay inserted until a new identifier is read on the serial port.
			if (d_insertedChip)
			{
				do
				{
					try
					{
						std::vector<unsigned char> cmd;
						cmd.push_back(0xff);	// trick

						std::vector<unsigned char> buf = getDefaultAdmittoReaderCardAdapter()->sendCommand(cmd);
						if (buf.size() > 0)
						{
							if (buf[0] == 0x4e)
							{
								INFO_SIMPLE_("Card removed !");
								d_insertedChip.reset();
								removed = true;
							}
							else
							{
								unsigned int l = static_cast<unsigned int>(atoull(BufferHelper::getStdString(buf)));
								char bufTmpId[64];
								sprintf(bufTmpId, "%08x", l);
								boost::shared_ptr<Chip> chip = ReaderUnit::createChip(
									(d_card_type == "UNKNOWN" ? "GenericTag" : d_card_type),
									formatHexString(std::string(bufTmpId))
								);

								if (chip->getChipIdentifier() != d_insertedChip->getChipIdentifier())
								{
									INFO_SIMPLE_("Card found but not same chip ! The previous card has been removed !");
									d_insertedChip.reset();
									removed = true;
								}
							}
						}
					}
					catch (std::exception&)
					{
						// No response received is ignored !
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
				} while (!removed && (maxwait == 0 || currentWait < maxwait));
			}
		}
		catch(...)
		{
			Settings::getInstance().IsLogEnabled = oldValue;
			throw;
		}

		INFO_("Returns card removed ? {%d} - function timeout expired ? {%d}", removed, (maxwait != 0 && currentWait >= maxwait));

		Settings::getInstance().IsLogEnabled = oldValue;

		return removed;
	}

	bool AdmittoReaderUnit::connect()
	{
		WARNING_SIMPLE_("Connect do nothing with Admitto reader");
		return true;
	}

	void AdmittoReaderUnit::disconnect()
	{
		WARNING_SIMPLE_("Disconnect do nothing with Admitto reader");
	}

	boost::shared_ptr<Chip> AdmittoReaderUnit::createChip(std::string type)
	{
		INFO_("Creating chip... chip type {0x%s(%s)}", type.c_str());
		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

		if (chip)
		{
			INFO_SIMPLE_("Chip created successfully !");
			boost::shared_ptr<ReaderCardAdapter> rca;
			boost::shared_ptr<CardProvider> cp;

			if (type == "GenericTag")
			{
				INFO_SIMPLE_("Generic tag Chip created");
				rca = getDefaultReaderCardAdapter();
				cp = LibraryManager::getInstance()->getCardProvider("GenericTag");
			}
			else
				return chip;

			rca->setReaderUnit(shared_from_this());
			chip->setCardProvider(cp);
		}
		return chip;
	}

	boost::shared_ptr<Chip> AdmittoReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > AdmittoReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<AdmittoReaderCardAdapter> AdmittoReaderUnit::getDefaultAdmittoReaderCardAdapter()
	{
		boost::shared_ptr<ReaderCardAdapter> adapter = getDefaultReaderCardAdapter();
		return boost::dynamic_pointer_cast<AdmittoReaderCardAdapter>(adapter);
	}

	std::string AdmittoReaderUnit::getReaderSerialNumber()
	{
		WARNING_SIMPLE_("Do nothing with Admitto reader");
		std::string ret;
		return ret;
	}

	bool AdmittoReaderUnit::isConnected()
	{
		if (d_insertedChip)
			INFO_SIMPLE_("Is connected {1}");
		else
			INFO_SIMPLE_("Is connected {0}");
		return bool(d_insertedChip);
	}

	bool AdmittoReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Connecting to reader...");
		bool ret = false;

		startAutoDetect();

		EXCEPTION_ASSERT_WITH_LOG(getSerialPort(), LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(getSerialPort()->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		if (!getSerialPort()->getSerialPort()->isOpen())
		{
			INFO_SIMPLE_("Serial port closed ! Opening it...");
			getSerialPort()->getSerialPort()->open();
			configure();
			ret = true;
		}
		else
		{
			INFO_SIMPLE_("Serial port already opened !");
			ret = true;
		}

		return ret;
	}

	void AdmittoReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Disconnecting from reader...");
		if (getSerialPort()->getSerialPort()->isOpen())
		{
			getSerialPort()->getSerialPort()->close();
		}
	}

	void AdmittoReaderUnit::startAutoDetect()
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

						boost::shared_ptr<AdmittoReaderUnit> testingReaderUnit(new AdmittoReaderUnit(*i));
						boost::shared_ptr<AdmittoReaderCardAdapter> testingCardAdapter(new AdmittoReaderCardAdapter());
						testingCardAdapter->setReaderUnit(testingReaderUnit);
						
						std::vector<unsigned char> cmd;
						cmd.push_back(0xff);	// trick

						std::vector<unsigned char> tmpASCIIId = testingCardAdapter->sendCommand(cmd, Settings::getInstance().AutoDetectionTimeout);

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

	void AdmittoReaderUnit::configure()
	{
		configure(getSerialPort(), Settings::getInstance().IsConfigurationRetryEnabled);
	}

	void AdmittoReaderUnit::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
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

	void AdmittoReaderUnit::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getReaderProvider()->getRPType());
		d_port->serialize(node);
		d_readerUnitConfig->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AdmittoReaderUnit::unSerialize(boost::property_tree::ptree& node)
	{
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
		d_readerUnitConfig->unSerialize(node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
	}

	boost::shared_ptr<AdmittoReaderProvider> AdmittoReaderUnit::getAdmittoReaderProvider() const
	{
		return boost::dynamic_pointer_cast<AdmittoReaderProvider>(getReaderProvider());
	}
}
