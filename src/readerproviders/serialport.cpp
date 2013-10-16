/**
 * \file serialport.cpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A serial port class.
 */

#include "logicalaccess/readerproviders/serialport.hpp"

#ifdef UNIX
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

/*
#include "system.hpp"
#include "invalid_call_exception.hpp"
#include "system_error_exception.hpp"*/

using std::string;


namespace logicalaccess
{
	SerialPort::SerialPort() :
#ifdef UNIX
		d_file(-1),
		d_dev("/dev/tty0")
#else
		d_file(INVALID_HANDLE_VALUE),
		d_dev("COM1")
#endif
	{
	}

	SerialPort::SerialPort(const string& dev) :
#ifdef UNIX
		d_file(-1),
#else
		d_file(INVALID_HANDLE_VALUE),
#endif
		d_dev(dev)
	{
	}

	void SerialPort::open()
	{
		if (isOpen()) return;

#ifdef UNIX
		d_file = ::open(d_dev.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
#else
		d_file = CreateFileA(("\\\\.\\" + d_dev).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		DWORD dwError = GetLastError();
		if (dwError == ERROR_FILE_NOT_FOUND)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Can't find the serial port.");
		}
#endif
	}

	void SerialPort::reopen()
	{
		close();
		open();
	}

	void SerialPort::close()
	{
		if (isOpen())
		{
#ifdef UNIX
			::close(d_file);
			d_file = -1;
#else
			CloseHandle(d_file);
			d_file = INVALID_HANDLE_VALUE;
#endif
		}
	}

#ifdef UNIX

	struct termios SerialPort::configuration() const
	{
		struct termios options;

		/* Get options */
		tcgetattr(d_file, &options);

		return options;
	}

	void SerialPort::setConfiguration(const struct termios& options)
	{
		/* Set the options */
		tcsetattr(d_file, TCSANOW, &options);
	}
#else

	DCB SerialPort::configuration() const
	{
		DCB options;
		options.DCBlength = sizeof(options);

		if (!GetCommState(d_file, &options))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot get the communication state.");
		}

		return options;
	}

	void SerialPort::setConfiguration(DCB& options)
	{
		if (!SetCommState(d_file, &options))
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot set the communication state.");
		}
	}
#endif

	size_t SerialPort::read(std::vector<unsigned char>& buf, size_t cnt) const
	{
		EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException, "Cannot read on a closed device");

		if (cnt == 0)
		{
			return 0;
		}

		buf.resize(cnt);

#ifdef UNIX
		ssize_t r = ::read(d_file, buf.data(), buf.size());

		if (r < 0)
		{
			if (errno != EAGAIN)
			{
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot read data.");
			}
			else
			{
				buf.resize(0);
				return 0;
			}
		}

		buf.resize(r);

		return r;
#else
		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 0;

		EXCEPTION_ASSERT(SetCommTimeouts(d_file, &timeouts), LibLogicalAccessException, "Cannot set serial port timeout");

		DWORD r = 0;
		BOOL result = ReadFile(d_file, buf.data(), static_cast<int>(buf.size()), &r, NULL);

		if (result == FALSE)
		{
			// DWORD er = GetLastError();
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot read data.");
			//EXCEPTION_THROW_LAST_SYSTEM_ERROR();
		}

		if (r > 0)
		{
			buf.resize(r);
		}
		else
		{
			buf.clear();
			r = 0;
		}

		if (d_readBuf.size() > 0)
		{
			buf.insert(buf.begin(), d_readBuf.begin(), d_readBuf.end());
			r += static_cast<int>(d_readBuf.size());
			d_readBuf.clear();
		}

		return r;
#endif

	}

	size_t SerialPort::write(const std::vector<unsigned char>& buf) const
	{
		EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException, "Cannot write on a closed device");

#ifdef UNIX
		ssize_t r = ::write(d_file, &buf[0], buf.size());

		if (r < 0)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot write data.");
		}

		return r;
#else
		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 0;

		DWORD r = 0;
		BOOL result = WriteFile(d_file, &buf[0], static_cast<DWORD>(buf.size()), &r, NULL);

		if (result != TRUE)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot write data.");
		}

		return r;
#endif
	}

	bool SerialPort::select(boost::posix_time::time_duration timeout)
	{
		EXCEPTION_ASSERT(isOpen(), LibLogicalAccessException, "Cannot select on a closed device");

		bool ret = false;

#ifdef UNIX

		fd_set rfs;
		FD_ZERO(&rfs);
		FD_SET(socketDescriptor(), &rfs);

		struct timeval tv;
		struct timeval* ptv = NULL;

		if (!timeout.is_special())
		{
			tv.tv_sec = static_cast<long>(timeout.total_seconds());
			tv.tv_usec = static_cast<long>(timeout.total_microseconds() - timeout.total_seconds() * 1000000L);
			ptv = &tv;
		}

		int retval;
		retval = ::select(socketDescriptor() + 1, &rfs, NULL, NULL, ptv);

		ret = (retval > 0);

		if (retval < 0)
		{
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot select the device.");
		}
#else

		// tell to unblock when 1 byte is ready
		if (SetCommMask(d_file, EV_RXCHAR))
		{
			COMMTIMEOUTS commTimeouts;

			// specify timeout
			if (GetCommTimeouts(d_file, &commTimeouts))
			{
				commTimeouts.ReadIntervalTimeout = MAXDWORD;
				commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
				commTimeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(timeout.total_microseconds() / 1000);
				commTimeouts.WriteTotalTimeoutMultiplier = 0;
				commTimeouts.WriteTotalTimeoutConstant = 0;

				// apply timeout
				if (SetCommTimeouts(d_file, &commTimeouts) == TRUE)
				{
					// wait at least 1 byte
					DWORD r = 0;
					d_readBuf.resize(1);
					BOOL result = ReadFile(d_file, &d_readBuf[0], static_cast<DWORD>(d_readBuf.size()), &r, NULL);
					if (result == FALSE)
					{
						d_readBuf.clear();
						THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot select the device.");
					}
					if (r < 1)
					{
						d_readBuf.clear();
					}
					else
					{
						d_readBuf.resize(r);
						ret = true;
					}
				}
			}
		}

#endif
		return ret;
	}
}

