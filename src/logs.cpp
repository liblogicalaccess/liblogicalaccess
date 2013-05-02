/**
 * \file logs.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Base class logs informations. 
 */
 
#include "logicalaccess/logs.h"

#include <string>
#include <sstream>
#include <cstdarg>

#include <boost/date_time.hpp>

namespace LOGICALACCESS
{
	std::ofstream Logs::logfile;

	std::string Logs::processLog(const char* file, int line, const char* function, const char* msg)
	{
		std::string msgStr;
		if (msg != NULL)
		{
			msgStr = std::string(msg);
		}
		std::string fileStr;
		if (file != NULL)
		{
			fileStr = std::string(file);
		}
		std::string functionStr;
		if (function != NULL)
		{
			functionStr = std::string(function);
		}
		std::ostringstream out;
		out << line;

		return std::string("\t{") + out.str() + std::string("}\t{") + functionStr + std::string("}\t{") + fileStr + std::string("}:\r\n\t") + msgStr + std::string("\r\n");
	}
	
	void Logs::DEBUG_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
//#ifdef DEBUG
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_DEBUG(processLog(file, line, function, buffer));
		va_end(args); 
//#endif
	}

	void Logs::ALERT_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_ALERT(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::CRITICAL_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_CRITICAL(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::EMERGENCY_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_EMERGENCY(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::ERROR_LOG(const char* file, int line, const char* function, const std::string& format)
	{
		Logs::ERROR_LOG(file, line, function, format.c_str());
	}

	void Logs::ERROR_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_ERROR(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::NOTICE_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_NOTICE(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::WARNING_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_WARNING(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::INFO_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (!LOGICALACCESS::Settings::getInstance().IsLogEnabled)
			return;

		char buffer[2048];
		memset(buffer, 0x00, sizeof(buffer));
		va_list args;
		va_start (args, format);
#ifdef __linux__
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
#else
		_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
#endif
		log_INFORMATIONAL(processLog(file, line, function, buffer));
		va_end(args); 
	}

	void Logs::COM_LOG(const char* file, int line, const char* function, const char* format, ...)
	{
		if (LOGICALACCESS::Settings::getInstance().IsLogEnabled && LOGICALACCESS::Settings::getInstance().SeeCommunicationLog)
		{
			char buffer[2048];
			memset(buffer, 0x00, sizeof(buffer));
			va_list args;
			va_start (args, format);
	#ifdef __linux__
			vsnprintf(buffer, sizeof(buffer) - 1, format, args);
	#else
			_vsnprintf_s(buffer, sizeof(buffer) - 1, format, args);
	#endif
			log_INFORMATIONAL(processLog(file, line, function, buffer));
			va_end(args);
		}
	}

	void Logs::log(std::string message)
	{
		if (logfile)
		{
			boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
			logfile << boost::posix_time::to_simple_string(now) << " - " << message << std::endl;
		}
	}
}

