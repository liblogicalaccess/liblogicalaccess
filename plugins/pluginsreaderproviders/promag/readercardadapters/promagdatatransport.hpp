/**
 * \file promagdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag DataTransport. 
 */

#ifndef SCIELDATATRANSPORT_HPP
#define SCIELDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "promagbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API PromagDataTransport : public SerialPortDataTransport
	{
	public:
		PromagDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

		virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
		{ d_port = port; d_port->getSerialPort()->setCircularBufferParser(new PromagBufferParser()); };

		/**
		 * \brief Get the transport type of this instance.
		 * \return The transport type.
		 */
		virtual std::string getTransportType() const { return "PromagSerialPort"; };

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
	      d_port->getSerialPort()->setCircularBufferParser(new PromagBufferParser()); }

		/**
		 * \brief Get the default Xml Node name for this object.
		 * \return The Xml node name.
		 */
		virtual std::string getDefaultXmlNodeName() const { return "PromagDataTransport"; };
	};

}

#endif /* SCIELDATATRANSPORT_HPP */

 
