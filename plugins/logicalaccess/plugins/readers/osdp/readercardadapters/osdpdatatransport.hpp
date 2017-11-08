/**
 * \file osdpdatatransport.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief OSDP DataTransport.
 */

#ifndef OSDPDATATRANSPORT_HPP
#define OSDPDATATRANSPORT_HPP

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/plugins/readers/osdp/readercardadapters/osdpbufferparser.hpp>

#include <string>
#include <vector>

namespace logicalaccess
{
class LIBLOGICALACCESS_API OSDPDataTransport : public SerialPortDataTransport
{
  public:
    explicit OSDPDataTransport(const std::string &portname = "")
        : SerialPortDataTransport(portname)
    {
    }

    void setSerialPort(std::shared_ptr<SerialPortXml> port) override
    {
        d_port = port;
        d_port->getSerialPort()->setCircularBufferParser(new OSDPBufferParser());
    }

    /**
 * \brief Get the transport type of this instance.
 * \return The transport type.
 */
    std::string getTransportType() const override
    {
        return "OSDPSerialPort";
    }

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override
    {
        boost::property_tree::ptree node;
        SerialPortDataTransport::serialize(node);
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override
    {
        SerialPortDataTransport::unSerialize(
            node.get_child(SerialPortDataTransport::getDefaultXmlNodeName()));
        d_port->getSerialPort()->setCircularBufferParser(new OSDPBufferParser());
    }

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override
    {
        return "OSDPDataTransport";
    }
};
}

#endif /* OSDPDATATRANSPORT_HPP */