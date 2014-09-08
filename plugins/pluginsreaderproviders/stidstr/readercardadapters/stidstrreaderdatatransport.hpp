/**
 * \file stidstrreaderdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief STidSTR DataTransport. 
 */

#ifndef STIDSTRREADERDATATRANSPORT_HPP
#define STIDSTRREADERDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "stidstrreaderbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API STidSTRDataTransport : public SerialPortDataTransport
	{
	public:
		STidSTRDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

		virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
		{ d_port = port; d_port->getSerialPort()->setCircularBufferParser(new STidSTRBufferParser()); };
	};

}

#endif /* STIDSTRREADERDATATRANSPORT_HPP */

 
