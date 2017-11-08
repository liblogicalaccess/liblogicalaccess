/**
 * \file serialportdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Serial port data transport for reader/card commands.
 */

#ifndef LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP
#define LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP

#include <logicalaccess/readerproviders/datatransport.hpp>
#include <logicalaccess/readerproviders/serialportxml.hpp>

namespace logicalaccess
{
#define TRANSPORT_SERIALPORT "SerialPort"

/**
 * \brief A serial port data transport class.
 */
class LIBLOGICALACCESS_API SerialPortDataTransport : public DataTransport
{
  public:
    /**
     * \brief Constructor.
     * \param portname The serial port name.
     */
    explicit SerialPortDataTransport(const std::string &portname = "");

    /**
     * \brief Destructor.
     */
    virtual ~SerialPortDataTransport();

    /**
     * \brief Get the transport type of this instance.
     * \return The transport type.
     */
    std::string getTransportType() const override
    {
        return TRANSPORT_SERIALPORT;
    }

    /**
     * \brief Configure the current serial port.
     */
    void configure() const;

    /**
     * \brief Configure the given serial port.
     * \param port The COM port to configure.
     * \param retryConfiguring On error, the function wait some milliseconds and retry
     * another time to configure the serial port.
     */
    void configure(std::shared_ptr<SerialPortXml> port, bool retryConfiguring) const;

    /**
     * \param Connect to the transport layer.
     * \return True on success, false otherwise.
     */
    bool connect() override;

    /**
     * \param Disconnect from the transport layer.
     */
    void disconnect() override;

    /**
     * \briaf Get if connected to the transport layer.
     * \return True if connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Get the data transport endpoint name.
     * \return The data transport endpoint name.
     */
    std::string getName() const override;

    /**
     * \brief Start to auto-detect the first serial port with a reader. Update serial port
     * when found.
     */
    virtual void startAutoDetect();

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
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Get the serial port baudrate.
     * \return The Baudrate.
     */
    unsigned long getPortBaudRate() const
    {
        return d_portBaudRate;
    }

    /**
     * \brief Set the serial port baudrate.
     * \param baudRate The Baudrate (9600, 19200, 38400, 57600, 115200).
     */
    void setPortBaudRate(unsigned long baudRate)
    {
        d_portBaudRate = baudRate;
    }

    std::shared_ptr<SerialPortXml> getSerialPort() const
    {
        return d_port;
    }

    virtual void setSerialPort(std::shared_ptr<SerialPortXml> port)
    {
        d_port = port;
    }

    void send(const ByteVector &data) override;

    ByteVector receive(long int timeout) override;

  protected:
    /**
     * \brief The auto-detected status
     */
    bool d_isAutoDetected;

    /**
     * \brief The serial port.
     */
    std::shared_ptr<SerialPortXml> d_port;

    /**
     * \brief The baudrate to use when configuring the serial port.
     */
    unsigned long d_portBaudRate;
};
}

#endif /* LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP */