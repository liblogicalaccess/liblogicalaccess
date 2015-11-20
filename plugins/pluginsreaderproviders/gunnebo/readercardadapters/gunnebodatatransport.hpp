/**
 * \file elatecdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Gunnebo DataTransport.
 */

#pragma once

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "gunnebobufferparser.hpp"

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API GunneboDataTransport : public SerialPortDataTransport
    {
    public:
        GunneboDataTransport(const std::string& portname = "") : SerialPortDataTransport(portname) { d_checksum = true; };

        virtual void setSerialPort(std::shared_ptr<SerialPortXml> port)
        {
            d_port = port;
            GunneboBufferParser* parser = new GunneboBufferParser();
            d_port->getSerialPort()->setCircularBufferParser(parser);
        };

        void setChecksum(bool checksum)
        {
            d_checksum = checksum;
        }

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const { return "GunneboSerialPort"; };

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        void serialize(boost::property_tree::ptree& parentNode)
        {
            boost::property_tree::ptree node;
            SerialPortDataTransport::serialize(node);
            node.put("Checksum", d_checksum);
            parentNode.add_child(getDefaultXmlNodeName(), node);
        }

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        void unSerialize(boost::property_tree::ptree& node)
        {
            SerialPortDataTransport::unSerialize(node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
            d_checksum = node.get_child("Checksum").get_value<bool>();
            GunneboBufferParser* parser = new GunneboBufferParser();
            d_port->getSerialPort()->setCircularBufferParser(parser);
        }

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const { return "GunneboDataTransport"; };

    protected:

        bool d_checksum;
    };
}
