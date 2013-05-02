/**
 * \file logs.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Logging header
 */

#ifndef LOGICALACCESS_LOGS_H
#define LOGICALACCESS_LOGS_H

/**
 * \brief Convenient alias to create an exception.
 */
#undef EXCEPTION
#define EXCEPTION(type, msg) type(/*__FUNCTION__ "," __FILE__ "," __LINE__ " : "*/ msg)

/**
 * \brief Assertion which raises an exception on failure.
 */
#define EXCEPTION_ASSERT(condition,type,msg) if (!(condition)) { throw EXCEPTION(type, msg); }

#ifdef LOGICALACCESS_LOGS

//#define PSTR(x)         PANTHEIOS_LITERAL_STRING(x)

#define DEBUG_(x, ...) LOGICALACCESS::Logs::DEBUG_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define ALERT_(x, ...) LOGICALACCESS::Logs::ALERT_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define CRITICAL_(x, ...) LOGICALACCESS::Logs::CRITICAL_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define EMERGENCY_(x, ...) LOGICALACCESS::Logs::EMERGENCY_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define ERROR_(x, ...) LOGICALACCESS::Logs::ERROR_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define NOTICE_(x, ...) LOGICALACCESS::Logs::NOTICE_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define WARNING_(x, ...) LOGICALACCESS::Logs::WARNING_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define INFO_(x, ...) LOGICALACCESS::Logs::INFO_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
#define COM_(x, ...) LOGICALACCESS::Logs::COM_LOG(__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)

/**
 * \brief Convenient alias to throw an exception with logs.
 */
#define THROW_EXCEPTION_WITH_LOG(type, msg) { ERROR_(msg); throw EXCEPTION(type, msg); }

/**
 * \brief Assertion which raises an exception on failure with logs.
 */
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

#else

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

#define THROW_EXCEPTION_WITH_LOG(type, msg) { throw EXCEPTION(type, msg); }
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg) if (!(condition)) { THROW_EXCEPTION_WITH_LOG(type, msg); }

#endif

#define DEBUG_SIMPLE_(x) DEBUG_(x, NULL)
#define ALERT_SIMPLE_(x) ALERT_(x, NULL)
#define CRITICAL_SIMPLE_(x) CRITICAL_(x, NULL)
#define EMERGENCY_SIMPLE_(x) EMERGENCY_(x, NULL)
#define ERROR_SIMPLE_(x) ERROR_(x, NULL)
#define NOTICE_SIMPLE_(x) NOTICE_(x, NULL)
#define WARNING_SIMPLE_(x) WARNING_(x, NULL)
#define INFO_SIMPLE_(x) INFO_(x, NULL)
#define COM_SIMPLE_(x) COM_(x, NULL)

#include "settings.h"
#include <fstream>

namespace LOGICALACCESS
{
	class LIBLOGICALACCESS_API Logs
	{
		public:
			static void DEBUG_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void ALERT_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void CRITICAL_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void EMERGENCY_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void ERROR_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void ERROR_LOG(const char* file, int line, const char* function, const std::string& msg);
			static void NOTICE_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void WARNING_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void INFO_LOG(const char* file, int line, const char* function, const char* format, ...);
			static void COM_LOG(const char* file, int line, const char* function, const char* format, ...);

			static std::ofstream logfile;

		private:
			static std::string processLog(const char* file, int line, const char* function, const char* msg);

			static void log_DEBUG(std::string message) { log("DEBUG: " + message); };
			static void log_ALERT(std::string message) { log("ALERT: " + message); };
			static void log_CRITICAL(std::string message) { log("CRITICAL: " + message); };
			static void log_EMERGENCY(std::string message) { log("EMERGENCY " + message); };
			static void log_ERROR(std::string message) { log("ERROR: " + message); };
			static void log_NOTICE(std::string message) { log("NOTICE: " + message); };
			static void log_WARNING(std::string message) { log("WARNING: " + message); };
			static void log_INFORMATIONAL(std::string message) { log("INFO: " + message); };

			static void log(std::string message);
	};
}

#endif
