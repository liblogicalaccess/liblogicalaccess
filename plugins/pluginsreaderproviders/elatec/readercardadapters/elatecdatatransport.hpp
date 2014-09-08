/**
 * \file elatecdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Elatec DataTransport. 
 */

#ifndef ELATECDATATRANSPORT_HPP
#define ELATECDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "elatecbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API ElatecDataTransport : public SerialPortDataTransport
	{
	public:
		ElatecDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

		virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
		{ d_port = port; d_port->getSerialPort()->setCircularBufferParser(new ElatecBufferParser()); };
	};

}

#endif /* ELATECDATATRANSPORT_HPP */

 
