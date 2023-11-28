/**
 * \file libusbdatatransport.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief LibUSB data transport.
 */

#include <logicalaccess/plugins/readers/libusb/libusbdatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ctime>
#include <libusb.h>

namespace logicalaccess
{
LibUSBDataTransport::LibUSBDataTransport()
    : DataTransport()
    , d_isConnected(false)
{
}

LibUSBDataTransport::~LibUSBDataTransport()
{
}

bool LibUSBDataTransport::connect()
{
    d_isConnected = true;
    return true;
}

void LibUSBDataTransport::disconnect()
{
    d_isConnected = false;
}

bool LibUSBDataTransport::isConnected()
{
    return d_isConnected;
}

std::string LibUSBDataTransport::getName() const
{
    return getLibUSBReaderUnit()->getName();
}

void LibUSBDataTransport::usb_write(const std::vector<unsigned char> &data, int report_type, int report_number)
{
    std::vector<unsigned char> buf = data;
    int r = libusb_claim_interface(getLibUSBReaderUnit()->getHandle(), 0);
    if (r == 0)
    {
        r = libusb_control_transfer(getLibUSBReaderUnit()->getHandle(),
                     LIBUSB_REQUEST_TYPE_CLASS |
                     LIBUSB_RECIPIENT_INTERFACE |
                     LIBUSB_ENDPOINT_OUT,
                     HID_SET_REPORT,
                     report_type << 8 | report_number, 0,
                     &buf[0], static_cast<int>(buf.size()),
                     1000);
        libusb_release_interface(getLibUSBReaderUnit()->getHandle(), 0);
        if (r > 0)
        {
            LOG(DEBUGS) << "Data written to USB interface.";
        }
        else
        {
            CheckLibUSBError(r);
        }
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Cannot claim usb interface for writing.");
    }
}

std::vector<unsigned char> LibUSBDataTransport::usb_read(int report_type, int report_number, int readlen, int timeout)
{
    int r = libusb_claim_interface(getLibUSBReaderUnit()->getHandle(), 0);
    if (r == 0)
    {
        unsigned char buf[256];
        memset(buf, 0x00, sizeof(buf));
        
        r = libusb_control_transfer(getLibUSBReaderUnit()->getHandle(),
                     LIBUSB_REQUEST_TYPE_CLASS |
                     LIBUSB_RECIPIENT_INTERFACE |
                     LIBUSB_ENDPOINT_IN,
                     HID_SET_REPORT,
                     report_type << 8 | report_number, 0,
                     buf, readlen,
                     timeout);

        libusb_release_interface(getLibUSBReaderUnit()->getHandle(), 0);
        if (r > 0)
        {
            LOG(DEBUGS) << "Data read from USB interface.";
        }
        else
        {
            CheckLibUSBError(r);
        }
        
        return std::vector<unsigned char>(buf, buf + readlen);
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Cannot claim usb interface for writing.");
    }
}

void LibUSBDataTransport::send(const std::vector<unsigned char> &data)
{
    usb_write(data, REPORT_TYPE_FEATURE, 0);
}

std::vector<unsigned char> LibUSBDataTransport::receive(long int timeout)
{
    return usb_read(REPORT_TYPE_FEATURE, 0, FEATURE_RPT_SIZE, static_cast<int>(timeout));
}

void LibUSBDataTransport::CheckLibUSBError(int errorFlag)
{
    if (errorFlag < 0)
    {
        char conv[64];
        std::string msg = std::string("LibUSB error : ");
        sprintf(conv, "%d", errorFlag);
        msg += std::string(conv);
        msg += std::string(". ");

        switch (errorFlag)
        {
        case LIBUSB_SUCCESS: msg += std::string("Success (no error)"); break;
        case LIBUSB_ERROR_IO: msg += std::string("Input/output error"); break;
        case LIBUSB_ERROR_INVALID_PARAM: msg += std::string("Invalid parameter"); break;
        case LIBUSB_ERROR_ACCESS: msg += std::string("Access denied (insufficient permissions)"); break;
        case LIBUSB_ERROR_NO_DEVICE: msg += std::string("No such device (it may have been disconnected)"); break;
        case LIBUSB_ERROR_NOT_FOUND: msg += std::string("Entity not found"); break;
        case LIBUSB_ERROR_BUSY: msg += std::string("Resource busy"); break;
        case LIBUSB_ERROR_TIMEOUT: msg += std::string("Operation timed out"); break;
        case LIBUSB_ERROR_OVERFLOW: msg += std::string("Overflow"); break;
        case LIBUSB_ERROR_PIPE: msg += std::string("Pipe error"); break;
        case LIBUSB_ERROR_INTERRUPTED: msg += std::string("System call interrupted (perhaps due to signal)"); break;
        case LIBUSB_ERROR_NO_MEM: msg += std::string("Insufficient memory"); break;
        case LIBUSB_ERROR_NOT_SUPPORTED: msg += std::string("Operation not supported or unimplemented on this platform"); break;
        case LIBUSB_ERROR_OTHER:
        default:
            msg += std::string("Other/unknown error");
            break;
        }

        THROW_EXCEPTION_WITH_LOG(CardException, msg);
    }
}

std::string LibUSBDataTransport::getDefaultXmlNodeName() const
{
    return "LibUSBDataTransport";
}

void LibUSBDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void LibUSBDataTransport::unSerialize(boost::property_tree::ptree & /*node*/)
{
}

std::vector<unsigned char>
LibUSBDataTransport::sendCommand(const std::vector<unsigned char> &command, long int timeout)
{
    LOG(LogLevel::COMS) << "Sending command " << BufferHelper::getHex(command)
                        << " command size {" << command.size() << "} timeout {" << timeout
                        << "}...";

    d_lastCommand = command;
    d_lastResult.clear();

    if (command.size() > 0)
        send(command);

    std::vector<unsigned char> res = receive(timeout);
    d_lastResult                   = res;
    LOG(LogLevel::COMS) << "Response received successfully ! Reponse: "
                        << BufferHelper::getHex(res) << " size {" << res.size() << "}";

    return res;
}
}
