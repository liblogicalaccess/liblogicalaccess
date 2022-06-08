/**
 * \file libusbdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LibUSB transport for reader/card commands.
 */

#ifndef LOGICALACCESS_LIBUSBDATATRANSPORT_HPP
#define LOGICALACCESS_LIBUSBDATATRANSPORT_HPP

#include <logicalaccess/readerproviders/datatransport.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <list>

namespace logicalaccess
{
#define TRANSPORT_LIBUSB "LibUSB"

#define	REPORT_TYPE_FEATURE 0x03
#define FEATURE_RPT_SIZE 8

/**
 * \brief An LibUSB data transport class.
 */
class LLA_READERS_LIBUSB_API LibUSBDataTransport : public DataTransport
{
  public:
    /**
     * \brief Constructor.
     */
    LibUSBDataTransport();

    /**
     * \brief Destructor.
     */
    virtual ~LibUSBDataTransport();

    /**
     * \brief Get the transport type of this instance.
     * \return The transport type.
     */
    std::string getTransportType() const override
    {
        return TRANSPORT_LIBUSB;
    }

    /**
 * \brief Get the default Xml Node name for this object.
 * \return The Xml node name.
 */
    std::string getDefaultXmlNodeName() const override;

    /**
    * \brief Serialize object to Xml Node.
    * \param parentNode The parent XML node.
    */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
    * \brief UnSerialize object from a Xml Node.
    * \param node The Xml node.
    */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
    * \brief Connect to the transport layer.
    * \return True on success, false otherwise.
    */
    bool connect() override;

    /**
    * \brief Disconnect from the transport layer.
    */
    void disconnect() override;

    /**
    * \briaf Get if connected to the transport layer.
    * \return True if connected, false otherwise.
    */
    bool isConnected() override;

    /**
    * \brief Get the LibUSB reader unit.
    * \return The LibUSB reader unit.
    */
    std::shared_ptr<LibUSBReaderUnit> getLibUSBReaderUnit() const
    {
        return std::dynamic_pointer_cast<LibUSBReaderUnit>(getReaderUnit());
    }

    /**
        * \brief Get the data transport endpoint name.
        * \return The data transport endpoint name.
        */
    std::string getName() const override;
    
    /**
     * \brief Send the data using rpleth protocol computation.
     * \param data The data to send.
     */
    virtual void send(const std::vector<unsigned char> &data) override;

    /**
     * \brief Receive data from reader.
     * \param timeout The time to wait data.
     * \return The data from reader.
     */
    virtual std::vector<unsigned char> receive(long int timeout = 5000) override;

    /**
     * \brief Send a command to the reader.
     * \param command The command buffer.
     * \param timeout The command timeout.
     * \return the result of the command.
     */
    std::vector<unsigned char> sendCommand(const std::vector<unsigned char> &command,
                                           long int timeout = 2000) override;
                                           
    void usb_write(const std::vector<unsigned char> &data, int report_type, int report_number);
    
    std::vector<unsigned char> usb_read(int report_type, int report_number, int readlen, int timeout = 1000);
    
    void CheckLibUSBError(int errorFlag);

  protected:
    bool d_isConnected;
};
}

#endif /* LOGICALACCESS_LIBUSBDATATRANSPORT_HPP */