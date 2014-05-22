/**
 * \file serialportdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Serial port data transport.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"


namespace logicalaccess
{
	SerialPortDataTransport::SerialPortDataTransport(const std::string& portname) : d_isAutoDetected(false)
	{
		d_port.reset(new SerialPortXml(portname));

#ifndef UNIX
		d_portBaudRate = CBR_9600;
#else
		d_portBaudRate = B9600;
#endif
	}

	SerialPortDataTransport::~SerialPortDataTransport()
	{
	}

	bool SerialPortDataTransport::connect()
	{
		bool ret = false;

		startAutoDetect();

		EXCEPTION_ASSERT_WITH_LOG(d_port, LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(d_port->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		if (!d_port->getSerialPort()->isOpen())
		{
			d_port->getSerialPort()->open();
			configure();
			ret = true;
		}
		else
		{
			ret = true;
		}

		return ret;
	}

	void SerialPortDataTransport::disconnect()
	{
		if (d_port->getSerialPort()->isOpen())
		{
			d_port->getSerialPort()->close();
		}
	}

	bool SerialPortDataTransport::isConnected()
	{
		return d_port->getSerialPort()->isOpen();
	}

	std::string SerialPortDataTransport::getName() const
	{
		std::string name;

		if (d_port)
		{
			name = d_port->getSerialPort()->deviceName();
		}

		return name;
	}

	void SerialPortDataTransport::send(const std::vector<unsigned char>& data)
	{
		if (data.size() > 0)
		{
			COM_("Send command: %s", BufferHelper::getHex(data).c_str());
			d_port->getSerialPort()->write(data);
		}
	}

	std::vector<unsigned char> SerialPortDataTransport::receive(long int timeout)
	{
		std::vector<unsigned char> res;

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout + 100);
		if (d_port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;

			while (d_port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());
			}
		}

		COM_("Command response: %s", BufferHelper::getHex(res).c_str());

		return res;
	}

	void SerialPortDataTransport::configure()
	{
		configure(d_port, Settings::getInstance()->IsConfigurationRetryEnabled);
	}

	void SerialPortDataTransport::configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring)
	{
		EXCEPTION_ASSERT_WITH_LOG(port, LibLogicalAccessException, "No serial port configured !");
		EXCEPTION_ASSERT_WITH_LOG(port->getSerialPort()->deviceName() != "", LibLogicalAccessException, "Serial port name is empty ! Auto-detect failed !");

		try
		{
			unsigned long baudrate = getPortBaudRate();

			DEBUG_("Configuring serial port %s - Baudrate {%ul}...", port->getSerialPort()->deviceName().c_str(), baudrate);

#ifndef _WINDOWS
			struct termios options = port->getSerialPort()->configuration();

			/* Set speed */
			cfsetispeed(&options, static_cast<speed_t>(baudrate));
			cfsetospeed(&options, static_cast<speed_t>(baudrate));

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
			options.BaudRate = baudrate;
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
							e.what(), Settings::getInstance()->ConfigurationRetryTimeout, portn.c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(Settings::getInstance()->ConfigurationRetryTimeout));

				port->getSerialPort()->reopen();
				configure(port, false);
			}
		}
	}

	void SerialPortDataTransport::startAutoDetect()
	{
		if (d_port && d_port->getSerialPort()->deviceName() == "")
		{
			if (!Settings::getInstance()->IsAutoDetectEnabled)
			{
				INFO_("Auto detection is disabled through settings !");
				return;
			}

			INFO_("Serial port is empty ! Starting Auto COM Port Detection...");
			std::vector<boost::shared_ptr<SerialPortXml> > ports;
			if (SerialPortXml::EnumerateUsingCreateFile(ports) && !ports.empty() && getReaderUnit())
			{
				std::vector<unsigned char> cmd = getReaderUnit()->getPingCommand();
				if (cmd.size() > 0)
				{
					boost::shared_ptr<ReaderCardAdapter> rca = getReaderUnit()->getDefaultReaderCardAdapter();
					std::vector<unsigned char> wrappedcmd = rca->adaptCommand(cmd);
					bool found = false;
					for (std::vector<boost::shared_ptr<SerialPortXml> >::iterator i  = ports.begin(); i != ports.end() && !found; ++i)
					{
						try
						{
							INFO_("Processing port {%s}...", (*i)->getSerialPort()->deviceName().c_str());
							(*i)->getSerialPort()->open();
							configure((*i), false);
						
							
							
							d_port = (*i);
							std::vector<unsigned char> r = sendCommand(wrappedcmd, Settings::getInstance()->AutoDetectionTimeout);
							if (r.size() > 0)
							{
								INFO_("Reader found ! Using this COM port !");
								found = true;
							}
						}
						catch (std::exception& e)
						{
							ERROR_("Exception {%s}", e.what());
						}

						if ((*i)->getSerialPort()->isOpen())
						{
							(*i)->getSerialPort()->close();
						}
					}

					if (!found)
					{
						INFO_("No reader found on COM port...");
					}
					else
					{
						d_isAutoDetected = true;
					}
				}
			}
			else
			{
				WARNING_("No COM Port detected !");
			}
		}
	}

	void SerialPortDataTransport::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getTransportType());
		node.put("PortBaudRate", d_portBaudRate);
		d_port->serialize(node);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SerialPortDataTransport::unSerialize(boost::property_tree::ptree& node)
	{
		d_portBaudRate = node.get_child("PortBaudRate").get_value<unsigned long>();
		d_port.reset(new SerialPortXml());
		d_port->unSerialize(node.get_child(d_port->getDefaultXmlNodeName()));
	}

	std::string SerialPortDataTransport::getDefaultXmlNodeName() const
	{
		return "SerialPortDataTransport";
	}
}
