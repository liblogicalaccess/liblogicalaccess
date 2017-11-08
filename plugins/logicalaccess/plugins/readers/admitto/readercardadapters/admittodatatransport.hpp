/**
 * \file admittobufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Admitto DataTransport.
 */

#ifndef ADMITTODATATRANSPORT_HPP
#define ADMITTODATATRANSPORT_HPP

#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/plugins/readers/admitto/readercardadapters/admittobufferparser.hpp>

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API AdmittoDataTransport : public SerialPortDataTransport
{
  public:
    explicit AdmittoDataTransport(const std::string &portname = "")
        : SerialPortDataTransport(portname)
    {
    }

    void setSerialPort(std::shared_ptr<SerialPortXml> port) override
    {
        d_port = port;
        d_port->getSerialPort()->setCircularBufferParser(new AdmittoBufferParser());
    }

    /**
 * \brief Get the transport type of this instance.
 * \return The transport type.
 */
    std::string getTransportType() const override
    {
        return "AdmittoSerialPort";
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
        d_port->getSerialPort()->setCircularBufferParser(new AdmittoBufferParser());
    }

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override
    {
        return "AdmittoDataTransport";
    }
};
}

#endif /* ADMITTODATATRANSPORT_HPP */