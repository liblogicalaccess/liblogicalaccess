/**
 * \file logs.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Base class logs informations.
 */

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    std::ofstream Logs::logfile;
    std::map<LogLevel, std::string> Logs::logLevelMsg;

    Logs::Logs(const char* file, const char* func, int line, enum LogLevel level) : d_level(level)
    {
        if (!Settings::getInstance()->IsLogEnabled
            || (d_level == LogLevel::COMS && !Settings::getInstance()->SeeCommunicationLog)
            || ((d_level == LogLevel::PLUGINS || d_level == LogLevel::PLUGINS_ERROR) && !Settings::getInstance()->SeePluginLog))
            d_level = NONE;

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

        if (logfile && d_level != NONE)
        {
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
            _stream << boost::posix_time::to_simple_string(now) << " - " << logLevelMsg[d_level] << ": \t{" << line << "}\t{" << func << "}\t{" << file << "}:" << std::endl;
        }
    }

    Logs::~Logs()
    {
        if (logfile && d_level != NONE)
        {
            _stream << std::endl;
            logfile << _stream.rdbuf();
            logfile.flush();
        }
    }
}