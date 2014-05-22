/**
 * \file logs.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Logging header
 */

#ifndef LOGICALACCESS_LOGS_HPP
#define LOGICALACCESS_LOGS_HPP

/**
 * \brief Convenient alias to create an exception.
 */
#undef EXCEPTION
#define EXCEPTION(type, msg) type(/*__FUNCTION__ "," __FILE__ "," __LINE__ " : "*/ msg)

/**
 * \brief Assertion which raises an exception on failure.
 */
#define EXCEPTION_ASSERT(condition,type,msg) if (!(condition)) { throw EXCEPTION(type, msg); }

#include "settings.hpp"
#include <fstream>
#include <map>
#include <boost/date_time.hpp>

namespace logicalaccess
{
	enum LogLevel { INFOS = 0, WARNINGS, NOTICES, ERRORS, EMERGENSYS, CRITICALS, ALERTS, DEBUGS, COMS, PLUGINS };

	class Logs
	{
	public:
		Logs(const char* file, const char* func, int line, enum LogLevel level)
		{
			if (!Settings::getInstance()->IsLogEnabled
			|| (level == LogLevel::COMS && !Settings::getInstance()->SeeCommunicationLog)
			|| (level == LogLevel::PLUGINS && !Settings::getInstance()->SeePluginLog))
				return;

			if (logLevelMsg.empty())
			{
				logLevelMsg[INFOS] = "INFO";
				logLevelMsg[WARNINGS] = "WARNING";
				logLevelMsg[NOTICES] = "NOTICE";
				logLevelMsg[ERRORS] = "ERROR";
				logLevelMsg[EMERGENSYS] = "EMERGENSY";
				logLevelMsg[CRITICALS] = "CRITICAL";
				logLevelMsg[ALERTS] = "ALERT";
				logLevelMsg[DEBUGS] = "DEBUG";
				logLevelMsg[COMS] = "COM";
				logLevelMsg[PLUGINS] = "PLUGIN";
			}

			if (logfile)
			{
				boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
				_stream << boost::posix_time::to_simple_string(now) << " - " << file << ": " << logLevelMsg[level] << line << ": in function '" << func << "': ";
			}
		}
   
		~Logs()
		{
			if (logfile)
			{
				_stream << std::endl;
				logfile << _stream.rdbuf();
				logfile.flush();
			}
		}
   
		template <class T>
		Logs& operator<<(const T& arg)
		{
			_stream << arg;
			return (*this);
		}
   
		static std::ofstream logfile;

	private:
		std::stringstream   _stream;
		static std::map<LogLevel, std::string> logLevelMsg;
	};

	#ifdef LOGICALACCESS_LOGS

	#define LOG(x) Logs(__FILE__, __FUNCTION__, __LINE__, x)

	/**
	 * \brief Convenient alias to throw an exception with logs.
	 */
	#define THROW_EXCEPTION_WITH_LOG(type, msg) { LOG(LogLevel::ERRORS) << msg; throw EXCEPTION(type, msg); }

	/**
	 * \brief Assertion which raises an exception on failure with logs.
	 */
	#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

	#else

	#define LOG(x) ((void 0)

	#define PSTR(x) ((void) 0)
	#define DEBUG_(x, ...) ((void) 0)
	#define ALERT_(x, ...) ((void) 0)
	#define CRITICAL_(x, ...) ((void) 0)
	#define EMERGENCY_(x, ...) ((void) 0)
	#define ERROR_(x, ...) ((void) 0)
	#define NOTICE_(x, ...) ((void) 0)
	#define WARNING_(x, ...) ((void) 0)
	#define INFO_(x, ...) ((void) 0)
	#define COM_(x, ...) ((void) 0)
	#define PLUGIN_(x, ...) ((void) 0)
	#define PLUGIN_ERROR_(x, ...) ((void) 0)

	#define THROW_EXCEPTION_WITH_LOG(type, msg) { throw EXCEPTION(type, msg); }
	#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

	#endif
}

#endif
