#pragma once

#include "logicalaccess/readerproviders/serialportdatatransport.hpp"

namespace logicalaccess
{

class STidPRGDataTransport : public SerialPortDataTransport
{
  public:
    virtual void setSerialPort(std::shared_ptr<SerialPortXml> port) override;

    STidPRGDataTransport();

    void setReceiveTimeout(long int t);

    long int getReceiveTimeout() const;

    /**
     * This overload IGNORES the timeout parameter.
     * It use the receiveTimeout_ attribute instead. The attribute
     * is controlable via the setReceiveTimeout() call.
     */
    virtual std::vector<unsigned char> receive(long int timeout) override;

    long int receiveTimeout_;
};
}
