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
		DIR* pdir = opendir("/dev");
		struct dirent *pent;
		if (pdir)
		{
			while (pent = readdir(pdir))
			{
				if ((strstr(pent->d_name, "ttyS") != 0) 
					|| (strstr(pent->d_name, "ttyUSB") != 0)
					|| (strstr(pent->d_name, "ttyACM") != 0))  
				{
				std::string p = ("/dev/");
				p.append(pent->d_name);

				try
				{
					boost::asio::io_service service;
					boost::asio::serial_port sp(service, portName);
					if (sp.is_open())
					{
						sp.close();
					 	boost::shared_ptr<SerialPortXml> newPort;
						newPort.reset(new SerialPortXml(portName));
						ports.push_back(newPort);
					}
				}
				catch (std::exception&)
				{
					LOG(LogLevel::ERRORS) << "Open SerialPort failed: " << p;
				}
			}
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Impossible to list SerialPort.");
#else
		bool success = false;
		bool stop = false;
		int bufferSize = 4096;
		while (bufferSize && !stop)
		{
			char *buffer = new char[bufferSize];
			int ret = QueryDosDevice(NULL, buffer, bufferSize);
			if (ret == 0)
			{
				DWORD dwError = GetLastError();
				if (dwError == ERROR_INSUFFICIENT_BUFFER)
					bufferSize *= 2;
				else
				{
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Impossible to list SerialPort.");
					stop = true;
				}
			}
			else
			{
				stop = true;
				success = true;
				size_t i = 0;
				while (buffer[i] != '\0')
				{
					char *currentDevice = buffer + i;
					size_t len = strlen(currentDevice);
					if (len > 3)
					{
						if (strncmp(currentDevice, "COM", 3) == 0)
						{
							if (std::all_of(currentDevice + 3, currentDevice + len, [](unsigned char c) { return ::isdigit(c); }))
							{
								boost::shared_ptr<SerialPortXml> newPort;
								newPort.reset(new SerialPortXml(std::string(currentDevice, currentDevice + len)));
								ports.push_back(newPort); 
							}
						}
					}

					i += len + 1;
				}
			}
		   delete buffer;
		}
#endif
		return success;
	}
}

