#pragma once

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{

class LIBLOGICALACCESS_API STidPRGDataTransport : public SerialPortDataTransport
{
  public:
    void setSerialPort(std::shared_ptr<SerialPortXml> port) override;

    STidPRGDataTransport();

    void setReceiveTimeout(long int t);

    long int getReceiveTimeout() const;

    /**
     * This overload IGNORES the timeout parameter.
     * It use the receiveTimeout_ attribute instead. The attribute
     * is controlable via the setReceiveTimeout() call.
     */
    ByteVector receive(long int timeout) override;

    /**
    * \brief Serialize the current object to XML.
    * \param parentNode The parent node.
    */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
    * \brief UnSerialize a XML node to the current object.
    * \param node The XML node.
    */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
    * \brief Get the default Xml Node name for this object.
    * \return The Xml node name.
    */
    std::string getDefaultXmlNodeName() const override
    {
        return "STidPRGDataTransport";
    }
    std::string getTransportType() const override
    {
        return "STidPRGSerialPort";
    }

    long int receiveTimeout_;
};
}
