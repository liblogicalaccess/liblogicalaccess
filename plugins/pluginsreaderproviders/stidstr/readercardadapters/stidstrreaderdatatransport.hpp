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

        virtual void setSerialPort(std::shared_ptr<SerialPortXml> port)
        {
            d_port = port; d_port->getSerialPort()->setCircularBufferParser(new STidSTRBufferParser());
        };

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const { return "STidSTRSerialPort"; };

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
            d_port->getSerialPort()->setCircularBufferParser(new STidSTRBufferParser());
        }

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const { return "STidSTRDataTransport"; };
    };
}

#endif /* STIDSTRREADERDATATRANSPORT_HPP */