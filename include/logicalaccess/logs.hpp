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
#define EXCEPTION(type, ...)                                                   \
    type(/*__FUNCTION__ "," __FILE__ "," __LINE__ " : "*/ __VA_ARGS__)

/**
 * \brief Assertion which raises an exception on failure.
 */
#define EXCEPTION_ASSERT(condition, type, msg)                                 \
    if (!(condition))                                                          \
    {                                                                          \
        throw EXCEPTION(type, msg);                                            \
    }

#include "logicalaccess_api.hpp"
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

namespace logicalaccess
{
    enum LogLevel
    {
        NONE = 0,
        TRACE,
        INFOS,
        WARNINGS,
        NOTICES,
        ERRORS,
        EMERGENSYS,
        CRITICALS,
        ALERTS,
        DEBUGS,
        COMS,
        PLUGINS,
        PLUGINS_ERROR
    };

    /**
     * An overload to pretty-print a byte vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &operator<<(std::ostream &ss, const std::vector<unsigned char> &bytebuff);

    /**
     * An overload to pretty-print a boolean vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &operator<<(std::ostream &ss, const std::vector<bool> &bytebuff);

    /**
     * A class that push a string into the current logger's context at
     * construction, and pop it at deletion.
     *
     * The direct use of this class is discouraged and the macro
     * LLA_LOG_CTX(...) should be used to push some context to the logger.
     */
    class LIBLOGICALACCESS_API LogContext
    {
      public:
        LogContext(const std::string &);
        ~LogContext();
    };

    class LIBLOGICALACCESS_API Logs
    {
      public:
        Logs(const char *file, const char *func, int line, enum LogLevel level);

        ~Logs();

        template <class T>
        Logs &operator<<(const T &arg)
        {
            _stream << arg;
            return (*this);
        }

        static std::ofstream logfile;

        /**
        * Do we duplicate the log to stderr?
        * Defaults to false.
        */
        static bool logToStderr;

      private:
        /**
         * Build a string containing some contextual information.
         */
        std::string pretty_context_infos();

        enum LogLevel d_level;
        std::stringstream _stream;
        static std::map<LogLevel, std::string> logLevelMsg;
    };

    /**
     * A RAII object that disable logging in its constructor, and restore
     * the old value in its destructor.
     *
     * This is used where we need to temporarily disable logging. This is
     * exception safe.
     */
    struct LIBLOGICALACCESS_API LogDisabler
    {
        LogDisabler();
        ~LogDisabler();

      private:
        bool old_;
    };

    /**
     * An overload to pretty-print a byte vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &
    operator<<(std::ostream &ss, const std::vector<unsigned char> &bytebuff);

    /**
     * An overload to pretty-print a boolean vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &operator<<(std::ostream &ss, const std::vector<bool> &bytebuff);

    LIBLOGICALACCESS_API std::ostream &
    operator<<(std::ostream &ss, const std::vector<bool> &bytebuff);

#ifdef LOGICALACCESS_LOGS

#define LOG(x) logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__, x)

	LIBLOGICALACCESS_API void trace_print_helper(std::stringstream &ss, const char *param_names, int idx);

	LIBLOGICALACCESS_API std::string get_nth_param_name(const char *param_names, int idx);

    /**
     * Declaration of the print_helper for non uint8_t types.
     */
    template<typename Current, typename ...T>
    typename std::enable_if<!std::is_same<unsigned char, typename std::remove_reference<Current>::type>::value>::type
    trace_print_helper(std::stringstream &ss, const char *param_names, int idx, Current &&c, T &&...args);

    /**
     * Declaration of the print_helper for uint8_t type.
     */
    template<typename Current, typename ...T>
    typename std::enable_if<std::is_same<unsigned char, typename std::remove_reference<Current>::type>::value>::type
    trace_print_helper(std::stringstream &ss, const char *param_names, int idx, Current &&c, T &&...args);


    template<typename Current, typename ...T>
    typename std::enable_if<std::is_same<unsigned char, typename std::remove_reference<Current>::type>::value>::type
    trace_print_helper(std::stringstream &ss, const char *param_names, int idx, Current &&c, T &&...args)
    {
      ss << " [" << get_nth_param_name(param_names, idx) << " --> " << +c << "]";
      trace_print_helper(ss, param_names, idx + 1, args...);
    }

    template<typename Current, typename ...T>
    typename std::enable_if<!std::is_same<unsigned char, typename std::remove_reference<Current>::type>::value>::type
    trace_print_helper(std::stringstream &ss, const char *param_names, int idx, Current &&c, T &&...args)
    {
      using namespace logicalaccess; // to access some custom ostream overload for vector.
      ss << " [" << get_nth_param_name(param_names, idx) << " --> " << c << "]";
      trace_print_helper(ss, param_names, idx + 1, args...);
    }

    template<typename ...T>
    void trace_print(std::stringstream &ss, const char *param_names, T &&...params)
    {
        trace_print_helper(ss, param_names, 0, params...);
    }

    /**
     * Log something at the TRACE level. This is a variadic macro that accepts all
     * parameters types and will output something like [param_name -> param_value]
     */
#define TRACE(...)                                                 \
  std::stringstream trace_stringstream;                            \
  trace_print(trace_stringstream, #__VA_ARGS__, ##__VA_ARGS__);    \
  LOG(TRACE) << trace_stringstream.str();

	LIBLOGICALACCESS_API void trace_print_helper(std::stringstream &ss, const char *param_names, int idx);

	LIBLOGICALACCESS_API std::string get_nth_param_name(const char *param_names, int idx);

#define LLA_LOG_CTX(param)                                                     \
    LogContext lla_log_ctx([&](void)                                           \
                           {                                                   \
                               std::stringstream logger_macro_ss__;            \
                               logger_macro_ss__ << param;                     \
                               return logger_macro_ss__.str();                 \
                           }())

/**
* Convenient alias to throw an exception with logs.
*/
#define THROW_EXCEPTION_WITH_LOG(type, msg, ...)                               \
    {                                                                          \
        LOG(logicalaccess::LogLevel::ERRORS) << msg;                           \
        throw EXCEPTION(type, msg, ##__VA_ARGS__);                             \
    }

/**
* Assertion which raises an exception on failure with logs.
*/
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg, ...)                   \
    if (!(condition))                                                          \
    {                                                                          \
        THROW_EXCEPTION_WITH_LOG(type, msg, ##__VA_ARGS__);                    \
    }

#else

#define LOG(x)                                                                 \
    logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__,                      \
                        logicalaccess::LogLevel::NONE)

#define THROW_EXCEPTION_WITH_LOG(type, msg)                                    \
    {                                                                          \
        throw EXCEPTION(type, msg);                                            \
    }
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg)                        \
    if (!(condition))                                                          \
    {                                                                          \
        THROW_EXCEPTION_WITH_LOG(type, msg);                                   \
    }

#endif
}

#endif
