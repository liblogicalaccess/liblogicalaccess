/**
 * \file logs.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Base class logs informations.
 */

#include <logicalaccess/logs.hpp>
#include <logicalaccess/settings.hpp>
#include <logicalaccess/colorize.hpp>
#include <boost/date_time.hpp>

#ifdef WIN32
// For now the additional context for the logger wont be thread safe,
// because the compiler doesn't support it.
static std::vector<std::string> context_;
#else
/**
 * A queue of "context string" to help make sense of the log message.
 */
static thread_local std::vector<std::string> context_;
#endif

namespace logicalaccess
{
bool Logs::logToStderr = false;
std::ofstream Logs::logfile;
std::map<LogLevel, std::string> Logs::logLevelMsg;

Logs::Logs(const char *file, const char *func, int line, enum LogLevel level)
    : d_level(level)
{
    if (!Settings::getInstance()->IsLogEnabled ||
        (d_level == COMS && !Settings::getInstance()->SeeCommunicationLog) ||
        ((d_level == PLUGINS || d_level == PLUGINS_ERROR) &&
         !Settings::getInstance()->SeePluginLog))
        d_level = NONE;

    if (logLevelMsg.empty())
    {
        logLevelMsg[INFOS]      = "INFO";
        logLevelMsg[WARNINGS]   = "WARNING";
        logLevelMsg[NOTICES]    = "NOTICE";
        logLevelMsg[ERRORS]     = "ERROR";
        logLevelMsg[EMERGENSYS] = "EMERGENSY";
        logLevelMsg[CRITICALS]  = "CRITICAL";
        logLevelMsg[ALERTS]     = "ALERT";
        logLevelMsg[DEBUGS]     = "DEBUG";
        logLevelMsg[COMS]       = "COM";
        logLevelMsg[PLUGINS]    = "PLUGIN";
    }

    if (logfile && d_level != NONE)
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        if (Settings::getInstance()->ColorizeLog)
        {
            _stream << Colorize::underline(to_simple_string(now)) << " - "
                    << Colorize::red(logLevelMsg[d_level]) << ": \t{" << line << "}\t{"
                    << Colorize::green(func) << "}\t{" << file << "}:" << std::endl;
        }
        else
        {
            _stream << to_simple_string(now) << " - " << logLevelMsg[d_level] << ": \t{"
                    << line << "}\t{" << func << "}\t{" << file << "}:" << std::endl;
        }
        if (Settings::getInstance()->ContextLog)
            _stream << pretty_context_infos();
    }
}

std::string Logs::pretty_context_infos()
{
    using namespace Colorize;
    if (context_.size() == 0)
        return "";

    std::string ret;
    if (Settings::getInstance()->ColorizeLog)
        ret = green(underline("Context:")) + ' ';
    else
        ret   = "Context: ";
    int count = 0;

    for (const auto &itr : context_)
    {
        if (count != 0)
            ret += std::string(9, ' ');
        std::stringstream ss;
        ss << count << ") ";
        if (Settings::getInstance()->ColorizeLog)
            ret += yellow(ss.str()) + itr + '\n';
        else
            ret += ss.str() + itr + '\n';
        count++;
    }
    return ret;
}

Logs::~Logs()
{
    if (logfile && d_level != NONE)
    {
        _stream << std::endl;
        logfile << _stream.rdbuf();
        logfile.flush();

        if (logToStderr)
            std::cerr << _stream.str();
    }
}

std::ostream &operator<<(std::ostream &ss, const ByteVector &bytebuff)
{
    std::stringstream tmp;
    tmp << "[data size: " << bytebuff.size() << "] : {";
    tmp << std::hex;
    for (auto itr = bytebuff.begin(); itr != bytebuff.end();)
    {
        tmp << (int)*itr;
        if (++itr != bytebuff.end())
            tmp << ", ";
    }
    tmp << "}";
    ss << tmp.str();
    return ss;
}

std::ostream &operator<<(std::ostream &ss, const std::vector<bool> &bytebuff)
{
    std::stringstream tmp;
    tmp << "[data size: " << bytebuff.size() << "] : {";
    tmp << std::hex;
    for (auto itr = bytebuff.begin(); itr != bytebuff.end();)
    {
        tmp << (int)*itr;
        if (++itr != bytebuff.end())
            tmp << ", ";
    }
    tmp << "}";
    ss << tmp.str();
    return ss;
}

LogDisabler::LogDisabler()
{
    old_                                  = Settings::getInstance()->IsLogEnabled;
    Settings::getInstance()->IsLogEnabled = false;
}

LogDisabler::~LogDisabler()
{
    Settings::getInstance()->IsLogEnabled = old_;
}

std::string get_nth_param_name(const char *param_names, int idx)
{
    // We probably can go full constexpr here.
    std::string ret;
    char *dup = strdup(param_names);
    char *ptr = strtok(dup, ", ");
    int i     = 1;

    if (idx == 0)
        ret = std::string(ptr);

    while ((ptr = strtok(nullptr, ", ")))
    {
        if (i == idx)
        {
            ret = std::string(ptr);
            break;
        }
        i++;
    }
    free(dup);
    return ret;
}

void trace_print_helper(std::stringstream &ss, const char *param_names, int /*idx*/)
{
    ss << std::endl;
}

LogContext::LogContext(const std::string &msg)
{
    context_.push_back(msg);
}

LogContext::~LogContext()
{
    context_.pop_back();
}
}
