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
	enum LogLevel { NONE = 0, INFOS, WARNINGS, NOTICES, ERRORS, EMERGENSYS, CRITICALS, ALERTS, DEBUGS, COMS, PLUGINS, PLUGINS_ERROR };

	class LIBLOGICALACCESS_API Logs
	{
	public:
		Logs(const char* file, const char* func, int line, enum LogLevel level);
   
		~Logs();
   
		template <class T>
		Logs& operator<<(const T& arg)
		{
			_stream << "\t" << arg;
			return (*this);
		}
   
		static std::ofstream logfile;

	private:
		enum LogLevel		d_level;
		std::stringstream   _stream;
		static std::map<LogLevel, std::string> logLevelMsg;
	};

	#ifdef LOGICALACCESS_LOGS

	#define LOG(x) logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__, x)

	/**
	 * \brief Convenient alias to throw an exception with logs.
	 */
	#define THROW_EXCEPTION_WITH_LOG(type, msg) { LOG(logicalaccess::LogLevel::ERRORS) << msg; throw EXCEPTION(type, msg); }

	/**
	 * \brief Assertion which raises an exception on failure with logs.
	 */
	#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

	#else

	#define LOG(x) logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__, Logs::LogLevel::NONE)

	#define THROW_EXCEPTION_WITH_LOG(type, msg) { throw EXCEPTION(type, msg); }
	#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

	#endif
}

#endif
