/**
 * \file serialport.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A serial port class.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/readerproviders/serialport.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <boost/asio.hpp>
#include <boost/asio/basic_serial_port.hpp>
#include <boost/bind/bind.hpp>
#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
SerialPort::SerialPort()
    :
#ifdef UNIX
    m_dev("/dev/tty0")
    ,
#else
    m_dev("COM1")
    ,
#endif
    m_serial_port(m_io)
    , m_circular_read_buffer(256)
    , m_read_buffer(128)
    , data_flag_(false)
{
}

SerialPort::SerialPort(const std::string &dev)
    : m_dev(dev)
    , m_serial_port(m_io)
    , m_circular_read_buffer(256)
    , m_read_buffer(128)
    , data_flag_(false)
{
}

void SerialPort::open()
{
    LOG(DEBUGS) << "Opening serial port...";

    if (m_serial_port.is_open())
        return;

    m_serial_port.open(m_dev);

    if (!m_serial_port.is_open())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Can't find the serial port.");

    if (!m_thread_reader)
    {
        data_flag_ = false;

        m_serial_port.async_read_some(
            boost::asio::buffer(m_read_buffer),
            boost::bind(&SerialPort::do_read, this, boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        m_thread_reader.reset(
            new std::thread(boost::bind(&boost::asio::io_service::run, &m_io)));
    }
}

void SerialPort::reopen()
{
    m_serial_port.close();
    m_serial_port.open(m_dev);
}

void SerialPort::close()
{
    m_io.post(boost::bind(&SerialPort::do_close, this, boost::system::error_code()));
    if (m_thread_reader)
        m_thread_reader->join();

    m_io.reset();
    m_thread_reader.reset();
    m_circular_read_buffer.clear();
    m_read_buffer.clear();
    m_read_buffer.resize(128);
    m_write_buffer.clear();
}

void SerialPort::do_close(const boost::system::error_code & /*error*/)
{
    if (m_serial_port.is_open())
    {
        m_serial_port.close();
    }
}

void SerialPort::setBaudrate(unsigned int rate)
{
    m_serial_port.set_option(boost::asio::serial_port_base::baud_rate(rate));
}

unsigned int SerialPort::getBaudrate()
{
    boost::asio::serial_port_base::baud_rate baud_rate;
    m_serial_port.get_option(baud_rate);
    return baud_rate.value();
}

void SerialPort::setFlowControl(
    const boost::asio::serial_port_base::flow_control::type &type)
{
    m_serial_port.set_option(boost::asio::serial_port_base::flow_control(type));
}

boost::asio::serial_port_base::flow_control::type SerialPort::getFlowControl()
{
    boost::asio::serial_port_base::flow_control flow_control;
    m_serial_port.get_option(flow_control);
    return flow_control.value();
}

void SerialPort::setParity(const boost::asio::serial_port_base::parity::type &parity)
{
    m_serial_port.set_option(boost::asio::serial_port_base::parity(parity));
}

boost::asio::serial_port_base::parity::type SerialPort::getParity()
{
    boost::asio::serial_port_base::parity parity;
    m_serial_port.get_option(parity);
    return parity.value();
}

void SerialPort::setStopBits(
    const boost::asio::serial_port_base::stop_bits::type &stop_bits)
{
    m_serial_port.set_option(boost::asio::serial_port_base::stop_bits(stop_bits));
}

boost::asio::serial_port_base::stop_bits::type SerialPort::getStopBits()
{
    boost::asio::serial_port_base::stop_bits stop_bits;
    m_serial_port.get_option(stop_bits);
    return stop_bits.value();
}

void SerialPort::setCharacterSize(unsigned int character_size)
{
    m_serial_port.set_option(
        boost::asio::serial_port_base::character_size(character_size));
}
unsigned int SerialPort::getCharacterSize()
{
    boost::asio::serial_port_base::character_size character_size;
    m_serial_port.get_option(character_size);
    return character_size.value();
}

size_t SerialPort::read(ByteVector &buf)
{
    EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException,
                     "Cannot read on a closed device");

    if (m_circular_buffer_parser)
    {
        buf = m_circular_buffer_parser->getValidBuffer(m_circular_read_buffer);
    }
    else
    {
        buf.assign(m_circular_read_buffer.begin(), m_circular_read_buffer.end());
        m_circular_read_buffer.clear();
        LOG(LogLevel::COMS) << "Use data read: " << BufferHelper::getHex(buf)
                            << " Size: " << buf.size();
    }

    return buf.size();
}

void SerialPort::do_read(const boost::system::error_code &error,
                         const size_t bytes_transferred)
{
    if (error == boost::asio::error::operation_aborted)
    {
        LOG(DEBUGS) << "Read aborted: " << error.message();
        return;
    }
    if (error == boost::asio::error::eof)
    {
        LOG(DEBUGS) << "Read errored (EOF)";
        do_close(error);
        return;
    }

    cond_var_mutex_.lock();
    if (m_circular_read_buffer.reserve() < bytes_transferred)
    {
        LOG(LogLevel::WARNINGS)
            << "Buffer Overflow, Size: " << m_circular_read_buffer.size()
            << " reserve: " << m_circular_read_buffer.reserve()
            << " bytes transferred: " << bytes_transferred;
        m_circular_read_buffer.clear();
    }
    m_circular_read_buffer.insert(m_circular_read_buffer.end(), m_read_buffer.begin(),
                                  m_read_buffer.begin() + bytes_transferred);

    LOG(LogLevel::INFOS) << "Data read: "
                         << BufferHelper::getHex(
                                ByteVector(m_read_buffer.begin(),
                                           m_read_buffer.begin() + bytes_transferred))
                         << " Size: " << bytes_transferred;

    data_flag_ = true;
    cond_var_mutex_.unlock();
    cond_var_.notify_all();

    // start the next read
    m_serial_port.async_read_some(
        boost::asio::buffer(m_read_buffer),
        boost::bind(&SerialPort::do_read, this, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

size_t SerialPort::write(const ByteVector &buf)
{
    EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException,
                     "Cannot write on a closed device");

    m_io.post(boost::bind(&SerialPort::do_write, this, buf));
    return buf.size();
}

void SerialPort::do_write(const ByteVector &buf)
{
    bool running = !m_write_buffer.empty();
    m_write_buffer.insert(m_write_buffer.end(), buf.begin(), buf.end());
    if (!running)
        write_start();
}

void SerialPort::write_start()
{
    async_write(m_serial_port, boost::asio::buffer(m_write_buffer),
                boost::bind(&SerialPort::write_complete, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}

void SerialPort::write_complete(const boost::system::error_code &error,
                                const size_t bytes_transferred)
{
    if (!error)
    { // write completed, so send next write data
        m_write_buffer.erase(m_write_buffer.begin(),
                             m_write_buffer.begin() + bytes_transferred);
        if (!m_write_buffer.empty())
            write_start();
    }
    else
        do_close(error);
}

bool SerialPort::isOpen() const
{
    return m_serial_port.is_open();
}

void SerialPort::dataConsumed()
{
    data_flag_ = false;
}
}