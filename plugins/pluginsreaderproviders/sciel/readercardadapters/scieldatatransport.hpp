/**
 * \file scieldatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel DataTransport. 
 */

#ifndef SCIELDATATRANSPORT_HPP
#define SCIELDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "scielbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API ScielDataTransport : public SerialPortDataTransport
	{
	public:
		ScielDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

		virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
		{ d_port = port; d_port->getSerialPort()->setCircularBufferParser(new ScielBufferParser()); };

		/**
		 * \brief Get the transport type of this instance.
		 * \return The transport type.
		 */
		virtual std::string getTransportType() const { return "ScielSerialPort"; };

		/**
		 * \brief Serialize the current object to XML.
		 * \param parentNode The parent node.
		 */
		void serialize(boost::property_tree::ptree& parentNode)
		{ boost::property_tree::ptree node;	SerialPortDataTransport::serialize(node); parentNode.add_child(getDefaultXmlNodeName(), node); }

		/**
		 * \brief UnSerialize a XML node to the current object.
		 * \param node The XML node.
		 */
		void unSerialize(boost::property_tree::ptree& node)
		{ SerialPortDataTransport::unSerialize(node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
	      d_port->getSerialPort()->setCircularBufferParser(new ScielBufferParser()); }

		/**
		 * \brief Get the default Xml Node name for this object.
		 * \return The Xml node name.
		 */
		virtual std::string getDefaultXmlNodeName() const { return "ScielDataTransport"; };
	};

}

#endif /* SCIELDATATRANSPORT_HPP */

 
