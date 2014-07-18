/**
 * \file serialportxml.cpp
 * \brief A serial port class which can be serialized.
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 */

#include "logicalaccess/readerproviders/serialportxml.hpp"

#ifdef UNIX
#include <fcntl.h>
#include <unistd.h>
#endif

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
	SerialPortXml::SerialPortXml()
	{
		d_serialport.reset(new SerialPort());
	}

	SerialPortXml::SerialPortXml(const std::string& dev)
	{
		d_serialport.reset(new SerialPort(dev));
	}	

	boost::shared_ptr<SerialPort> SerialPortXml::getSerialPort() const
	{
		return d_serialport;
	}

	void SerialPortXml::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("Device", d_serialport->deviceName());

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void SerialPortXml::unSerialize(boost::property_tree::ptree& node)
	{
		if (d_serialport->isOpen())
		{
			d_serialport->close();
		}
		
		std::string port = node.get_child("Device").get_value<std::string>();

		d_serialport.reset(new SerialPort(port));
	}	

	std::string SerialPortXml::getDefaultXmlNodeName() const
	{
		return "SerialPort";
	}

	bool SerialPortXml::EnumerateUsingCreateFile(std::vector<boost::shared_ptr<SerialPortXml> >& ports)
	{
		ports.clear();

#ifdef UNIX
		boost::shared_ptr<SerialPortXml> port0;
		port0.reset(new SerialPortXml("/dev/ttyS0"));
		boost::shared_ptr<SerialPortXml> port1;
		port1.reset(new SerialPortXml("/dev/ttyS1"));
		boost::shared_ptr<SerialPortXml> port2;
		port2.reset(new SerialPortXml("/dev/ttyS2"));
		boost::shared_ptr<SerialPortXml> port3;
		port3.reset(new SerialPortXml("/dev/ttyS3"));
		boost::shared_ptr<SerialPortXml> port4;
		port4.reset(new SerialPortXml("/dev/ttyS4"));
		boost::shared_ptr<SerialPortXml> port5;
		port5.reset(new SerialPortXml("/dev/ttyS5"));
#else

		// Up to 255 COM ports are supported so we iterate through all of them seeing
		// if we can open them or if we fail to open them, get an access denied or general error error.
		// Both of these cases indicate that there is a COM port at that number. 
		boost::asio::io_service io;
		boost::asio::serial_port port(io);
		for (unsigned int i = 1; i < 256; i++)
		{
			// Form the Raw device name
			char portname[64];
			sprintf(portname, "COM%u", i);


			try
			{
				// create the serial device, note it takes the io service and the port name
				port.open(portname);
				port.close();

				boost::shared_ptr<SerialPortXml> newPort;
				newPort.reset(new SerialPortXml(portname));
				ports.push_back(newPort); 
			}
			catch (std::exception& e)
			{
				LOG(LogLevel::WARNINGS) << portname << ": " << e.what();
			}
		}
		
#endif
		return true;
	}
}

