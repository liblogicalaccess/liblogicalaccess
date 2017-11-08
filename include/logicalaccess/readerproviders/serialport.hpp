/**
 * \file serialport.hpp
 * \author Julien KAUFFMANN <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A serial port class.
 */

#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP

#include <iostream>
#include <string>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <condition_variable>

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/readerproviders/circularbufferparser.hpp>

namespace logicalaccess
{
/**
 * \brief A class that represents a serial (COM) port.
 *
 * SerialPort is a simple class that help dealing with serial port in a platform
 * independant manner.
 *
 * The serial port must open after construction. After it has been opened, a SerialPort
 * should be configured using setConfiguration().
 *
 * Then you can either read() or write() data, or simply wait for it using select().
 */
class LIBLOGICALACCESS_API SerialPort
{
  public:
    /**
     * \brief Constructor.
     *
     * Tries to open "COM1" on Windows, "/dev/tty0" on Linux.
     */
    SerialPort();

    /**
     * \brief Constructor.
     * \param dev The device name. (Example: "/dev/tty0")
     */
    explicit SerialPort(const std::string &dev);

    /**
    * \brief Remove copy.
    */
    SerialPort(const SerialPort &other) = delete;       // non construction-copyable
    SerialPort &operator=(const SerialPort &) = delete; // non copyable

    /**
     * \brief Destructor.
     * \see close()
     * Automatically closes the serial port. See close().
     */
    virtual ~SerialPort()
    {
        close();
    }

    /**
     * \brief Open the serial port.
     * \see close()
     * \see reopen()
     * \see getConfiguration()
     * \see setConfiguration()
     *
     * Once the serial port is open, you must configure the serial port before any I/O
     * operation.
     *
     * If the open fails, a std::exception is thrown to give more information about the
     * failure.
     *
     * Attempting to open() a SerialPort for which isOpen() returns true has no effect.
     */
    void open();

    /**
     * \brief Reopen the serial port.
     * \see open()
     * \see close()
     *
     * Convenience method. reopen() has the exact same behavior than a call to close()
     * followed by a call to open().
     */
    void reopen();

    /**
     * \brief Close the serial port.
     * \see open()
     * \see reopen()
     *
     * Calling close() on an already closed serial port has no effect.
     */
    void close();

    bool isOpen() const;

    /**
     * \brief Get the device name.
     * \return The device name.
     */
    const std::string &deviceName() const
    {
        return m_dev;
    }

    /**
     * \brief Read some data from the serial port.
     * \param buf The buffer into which the new data must be stored.
     * \param cnt The count of bytes to read. If cnt is 0, the call with return immediatly
     * with a return value of 0.
     * \return The count of read bytes. buf.size() is also set, accordingly. Return value
     * is 0 if no data is available.
     * \warning If buf contains data, it will be discarded, even if the call fails.
     *
     * The serial port must be open before the call or an LibLogicalAccessException will
     * be thrown.
     * The call may also throw a std::exception in case of failure.
     */
    size_t read(ByteVector &buf);

    /**
     * \brief Write some data to the serial port.
     * \param buf The data to write to the serial port.
     * \return The count of bytes written.
     *
     * The serial port must be open before the call or an LibLogicalAccessException will
     * be thrown.
     * The call may also throw a std::exception in case of failure.
     */
    size_t write(const ByteVector &buf);

    void setBaudrate(unsigned int rate);
    unsigned int getBaudrate();

    void setFlowControl(const boost::asio::serial_port_base::flow_control::type &type);
    boost::asio::serial_port_base::flow_control::type getFlowControl();

    void setParity(const boost::asio::serial_port_base::parity::type &parity);
    boost::asio::serial_port_base::parity::type getParity();

    void setStopBits(const boost::asio::serial_port_base::stop_bits::type &stop_bits);
    boost::asio::serial_port_base::stop_bits::type getStopBits();

    void setCharacterSize(unsigned int character_size);
    unsigned int getCharacterSize();

    void setCircularBufferParser(CircularBufferParser *circular_buffer_parser)
    {
        m_circular_buffer_parser.reset(circular_buffer_parser);
    }
    std::shared_ptr<CircularBufferParser> getCircularBufferParser() const
    {
        return m_circular_buffer_parser;
    }

    boost::circular_buffer<unsigned char> &getCircularReadBuffer()
    {
        return m_circular_read_buffer;
    }

    /**
     * Wait until more data are available, or until `until` is reach.
     *
     * If more data are available, execute the callback while holding
     * the internal mutex.
     */
    template <typename T>
    void waitMoreData(const std::chrono::steady_clock::time_point &until, T &&callback)
    {
        std::unique_lock<std::mutex> ul(cond_var_mutex_);
        cond_var_.wait_until(ul, until, [&]() { return data_flag_; });
        if (data_flag_)
        {
            callback();
        }
    }

    /**
     * Execute `callback` while holding the internal mutex.
     */
    template <typename T>
    void lockedExecute(T &&callback)
    {
        std::unique_lock<std::mutex> ul(cond_var_mutex_);
        callback();
    }

    /**
     * Calling this function means that available data have been consumed,
     * or that more data is needed to continue.
     */
    void dataConsumed();

  private:
    void do_read(const boost::system::error_code &e, size_t bytes_transferred);

    void do_close(const boost::system::error_code &error);

    void do_write(const ByteVector &buf);
    void write_start();
    void write_complete(const boost::system::error_code &error,
                        const size_t bytes_transferred);

    /**
 * \brief The internal device name.
 */
    std::string m_dev;

    boost::asio::io_service m_io;

    boost::asio::serial_port m_serial_port;

    boost::circular_buffer<unsigned char> m_circular_read_buffer;

    ByteVector m_read_buffer;

    ByteVector m_write_buffer;

    std::shared_ptr<std::thread> m_thread_reader;

    std::shared_ptr<CircularBufferParser> m_circular_buffer_parser;

    /**
     * Synchronization stuff
     */

    std::condition_variable cond_var_;
    bool data_flag_;
    std::mutex cond_var_mutex_;
};
}

#endif /* SERIALPORT_HPP */