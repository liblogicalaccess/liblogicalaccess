/**
 * \file serialport.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A serial port class.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/readerproviders/serialport.hpp"

#include <boost/asio.hpp>
#include <boost/asio/basic_serial_port.hpp>

namespace logicalaccess
{
	SerialPort::SerialPort() :
#ifdef UNIX
		m_dev("/dev/tty0"),
#else
		m_dev("COM1"),
#endif
		m_serial_port(m_io), m_timeout(50)
	{
	}

	SerialPort::SerialPort(const std::string& dev) : m_dev(dev), m_serial_port(m_io), m_timeout(50)
	{
	}

	void SerialPort::open()
	{
		if (m_serial_port.is_open())
			return;

		m_serial_port.open(m_dev);

		if (!m_serial_port.is_open())
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't find the serial port.");
	}

	void SerialPort::reopen()
	{
		m_serial_port.close();
		m_serial_port.open(m_dev);
	}

	void SerialPort::close()
	{
		m_io.stop();
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

	void SerialPort::setFlowControl(const boost::asio::serial_port_base::flow_control::type& type)
	{
		m_serial_port.set_option(boost::asio::serial_port_base::flow_control(type));
	}

	boost::asio::serial_port_base::flow_control::type SerialPort::getFlowControl()
	{
		boost::asio::serial_port_base::flow_control flow_control;
		m_serial_port.get_option(flow_control);
		return flow_control.value();
	}

	void SerialPort::setParity(const boost::asio::serial_port_base::parity::type& parity)
	{
		m_serial_port.set_option(boost::asio::serial_port_base::parity(parity));
	}

	boost::asio::serial_port_base::parity::type SerialPort::getParity()
	{
		boost::asio::serial_port_base::parity parity;
		m_serial_port.get_option(parity);
		return parity.value();
	}

	void SerialPort::setStopBits(const boost::asio::serial_port_base::stop_bits::type& stop_bits)
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
		m_serial_port.set_option(boost::asio::serial_port_base::character_size(character_size));
	}
	unsigned int SerialPort::getCharacterSize()
	{
		boost::asio::serial_port_base::character_size character_size;
		m_serial_port.get_option(character_size);
		return character_size.value();
	}

	size_t SerialPort::read(std::vector<unsigned char>& buf, size_t cnt)
	{
		EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException, "Cannot read on a closed device");
		if (cnt == 0)
		{
			return 0;
		}

		buf.resize(cnt);

		boost::asio::deadline_timer timeout(m_io);
		unsigned char my_buffer[256];
		std::size_t data_available = 0;

		m_serial_port.async_read_some(boost::asio::buffer(my_buffer),
						boost::bind(&SerialPort::read_callback, this, boost::ref(data_available), boost::ref(timeout),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
		timeout.expires_from_now(boost::posix_time::milliseconds(m_timeout));
		timeout.async_wait(boost::bind(&SerialPort::wait_callback, this, boost::ref(m_serial_port),
						boost::asio::placeholders::error));

		m_io.run();  // will block until async callbacks are finished
		if (data_available)
			buf = std::vector<unsigned char>(my_buffer, my_buffer + data_available);
		else
			buf.clear();
		m_io.reset();
		return data_available;
	}

	void SerialPort::read_callback(std::size_t& data_available, boost::asio::deadline_timer& timeout, const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if (error || !bytes_transferred) // No data was read!
			return;
		data_available = bytes_transferred;
		timeout.cancel();  // will cause wait_callback to fire with an error
	}

	void SerialPort::wait_callback(boost::asio::serial_port& m_serial_port, const boost::system::error_code& error)
	{
		if (!error)
			m_serial_port.cancel(); // will cause read_callback to fire with an error
	}

	size_t SerialPort::write(const std::vector<unsigned char>& buf)
	{
		EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException, "Cannot write on a closed device");

		return m_serial_port.write_some(boost::asio::buffer(buf));
	}

	bool SerialPort::isOpen()
	{
		return m_serial_port.is_open();
	}
}