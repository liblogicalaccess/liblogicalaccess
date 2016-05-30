/**
 * \file gigatmsdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief GIGA-TMS DataTransport.
 */

#ifndef SCIELDATATRANSPORT_HPP
#define SCIELDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "gigatmsbufferparser.hpp"

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API GigaTMSDataTransport : public SerialPortDataTransport
    {
    public:
		GigaTMSDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

        virtual void setSerialPort(std::shared_ptr<SerialPortXml> port)
        {
            d_port = port; d_port->getSerialPort()->setCircularBufferParser(new GigaTMSBufferParser());
        };

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const { return "GigaTMSSerialPort"; };

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        void serialize(boost::property_tree::ptree& parentNode)
        {
            boost::property_tree::ptree node;	SerialPortDataTransport::serialize(node); parentNode.add_child(getDefaultXmlNodeName(), node);
        }

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        void unSerialize(boost::property_tree::ptree& node)
        {
            SerialPortDataTransport::unSerialize(node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
            d_port->getSerialPort()->setCircularBufferParser(new GigaTMSBufferParser());
        }

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const { return "GigaTMSDataTransport"; };
    };
}

#endif /* SCIELDATATRANSPORT_HPP */