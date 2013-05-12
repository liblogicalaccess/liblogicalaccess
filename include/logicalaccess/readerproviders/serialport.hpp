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

#ifdef UNIX
# include <termios.h>
# include <unistd.h>
#else
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# include <WinBase.h>
#endif

#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "logicalaccess/readerproviders/readerunit.hpp"

namespace logicalaccess
{
	/**
	 * \brief A class that represents a serial (COM) port.
	 *
	 * SerialPort is a simple class that help dealing with serial port in a platform independant manner.
	 *
	 * The serial port must open after construction. After it has been opened, a SerialPort should be configured using setConfiguration().
	 *
	 * Then you can either read() or write() data, or simply wait for it using select().
	 */
	class LIBLOGICALACCESS_API SerialPort :  public boost::noncopyable
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
			SerialPort(const std::string& dev);

			/**
			 * \brief Destructor.
			 * \see close()
			 * Automatically closes the serial port. See close().
			 */
			virtual inline ~SerialPort()
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
			 * Once the serial port is open, you must configure the serial port before any I/O operation.
			 *
			 * If the open fails, a std::exception is thrown to give more information about the failure.
			 *
			 * Attempting to open() a SerialPort for which isOpen() returns true has no effect.
			 */
			void open();

			/**
			 * \brief Reopen the serial port.
			 * \see open()
			 * \see close()
			 *
			 * Convenience method. reopen() has the exact same behavior than a call to close() followed by a call to open().
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

			/**
			 * \brief Get the device name.
			 * \return The device name.
			 */
			inline const std::string& deviceName() const
			{
				return d_dev;
			}

#ifdef UNIX

			/**
			 * \brief Check if the serial port is open.
			 * \return true if the serial port is open, false otherwise.
			 */
			inline bool isOpen() const
			{
				return (d_file >= 0);
			}

			/**
			 * \brief Get the internal socket descriptor.
			 * \return The internal socket descriptor.
			 */
			inline int socketDescriptor() const
			{
				return d_file;
			}

			/**
			 * \brief Get the serial port configuration.
			 * \return The serial port configuration.
			 */
			struct termios configuration() const;

			/**
			 * \brief Set the serial port configuration.
			 * \param options The new configuration.
			 */
			void setConfiguration(const struct termios& options);

#else
			/**
			 * \brief Check if the serial port is open.
			 * \return true if the serial port is open, false otherwise.
			 */
			inline bool isOpen() const
			{
				return (d_file != INVALID_HANDLE_VALUE);
			}

			/**
			 * \brief Get the internal file handle.
			 * \return The internal file handle.
			 */
			inline HANDLE fileHandle() const
			{
				return d_file;
			}

			/**
			 * \brief Get the serial port configuration.
			 * \return The serial port configuration.
			 */
			DCB configuration() const;

			/**
			 * \brief Set the serial port configuration.
			 * \param options The new configuration.
			 */
			void setConfiguration(DCB& options);
#endif

			/**
			 * \brief Read some data from the serial port.
			 * \param buf The buffer into which the new data must be stored.
			 * \param cnt The count of bytes to read. If cnt is 0, the call with return immediatly with a return value of 0.
			 * \return The count of read bytes. buf.size() is also set, accordingly. Return value is 0 if no data is available.
			 * \warning If buf contains data, it will be discarded, even if the call fails.
			 *
			 * The serial port must be open before the call or an LibLogicalAccessException will be thrown.
			 * The call may also throw a std::exception in case of failure.
			 */
			size_t read(std::vector<unsigned char>& buf, size_t cnt) const;

			/**
			 * \brief Write some data to the serial port.
			 * \param buf The data to write to the serial port.
			 * \return The count of bytes written.
			 *
			 * The serial port must be open before the call or an LibLogicalAccessException will be thrown.
			 * The call may also throw a std::exception in case of failure.
			 */
			size_t write(const std::vector<unsigned char>& buf) const;

			/**
			 * \brief Wait for something to read or maximum timeout milliseconds.
			 * \param timeout The I/O timeout, in milliseconds. If timeout is a special value, the call wait forever.
			 * \return True if no error, false on timeout.
			 */
			bool select(boost::posix_time::time_duration timeout);

		private:

#ifdef UNIX
			/**
			 * \brief The internal file descriptor.
			 */
			int d_file;
#else
			/**
			 * \brief The internal file handle.
			 */
			HANDLE d_file;

			/**
			 * \brief Buffer use to read the first byte.
			 */
			mutable std::vector<unsigned char> d_readBuf;
#endif

			/**
			 * \brief The internal device name.
			 */
			std::string d_dev;
	};
}

#endif /* SERIALPORT_HPP */
