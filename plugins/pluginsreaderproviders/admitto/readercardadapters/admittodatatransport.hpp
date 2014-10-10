/**
 * \file promagdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Admitto DataTransport.
 */

#ifndef ADMITTODATATRANSPORT_HPP
#define ADMITTODATATRANSPORT_HPP

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "admittobufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API AdmittoDataTransport : public SerialPortDataTransport
    {
    public:
        AdmittoDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) {};

        virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port)
        {
            d_port = port; d_port->getSerialPort()->setCircularBufferParser(new AdmittoBufferParser());
        };

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const { return "AdmittoSerialPort"; };

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
            d_port->getSerialPort()->setCircularBufferParser(new AdmittoBufferParser());
        }

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const { return "AdmittoDataTransport"; };
    };
}

#endif /* ADMITTODATATRANSPORT_HPP */