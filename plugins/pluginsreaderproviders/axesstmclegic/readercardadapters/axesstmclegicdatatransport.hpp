/**
 * \file axesstmclegicdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief AxessTMCLegic DataTransport.
 */

#ifndef AXESSTMCLEGICDATATRANSPORT_HPP
#define AXESSTMCLEGICDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "axesstmclegicbufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API AxessTMCLegicDataTransport : public SerialPortDataTransport
    {
    public:
        AxessTMCLegicDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

        virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
        {
            d_port = port; d_port->getSerialPort()->setCircularBufferParser(new AxessTMCLegicBufferParser());
        };

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const { return "AxessTMCLegicSerialPort"; };

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
            d_port->getSerialPort()->setCircularBufferParser(new AxessTMCLegicBufferParser());
        }

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const { return "AxessTMCLegicDataTransport"; };
    };
}

#endif /* AXESSTMCLEGICDATATRANSPORT_HPP */