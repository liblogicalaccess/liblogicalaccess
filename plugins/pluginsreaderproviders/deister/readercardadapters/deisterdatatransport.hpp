/**
 * \file deisterdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Deister DataTransport. 
 */

#ifndef DEISTERDATATRANSPORT_HPP
#define DEISTERDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "deisterbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API DeisterDataTransport : public SerialPortDataTransport
	{
	public:
		DeisterDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

		virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
		{ d_port = port; d_port->getSerialPort()->setCircularBufferParser(new DeisterBufferParser()); };
	};

}

#endif /* DEISTERDATATRANSPORT_HPP */

 
