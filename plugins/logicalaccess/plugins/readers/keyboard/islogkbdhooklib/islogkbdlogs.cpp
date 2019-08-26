#include "stdafx.h"

#include "islogkbdlogs.hpp"
#include "islogkbdsettings.hpp"

namespace islogkbdlib
{
KbdLogs *KbdLogs::instance    = NULL;
FILE *KbdLogs::currentLogFile = NULL;

KbdLogs::KbdLogs()
{
}

KbdLogs *KbdLogs::getInstance()
{
    if (instance == NULL)
    {
        instance = new KbdLogs();
        instance->Initialize();
    }
    return instance;
}

void KbdLogs::Initialize()
{
    static bool alreadyInit = false;

    if (alreadyInit == true)
        return;
    alreadyInit = true;

    try
    {
        OutputDebugStringA("#KbdLogs::Initialize# begin.");

        if (islogkbdlib::KbdSettings::getInstance()->IsLogEnabled)
        {
            CloseLogFile();

            std::string fullPath = islogkbdlib::KbdSettings::getInstance()->getDllPath() +
                                   "\\" +
                                   islogkbdlib::KbdSettings::getInstance()->LogFileName;

            OpenLogFile(fullPath.c_str());
        }
    }
    catch (...)
    {
    }
}

void KbdLogs::Uninitialize()
{
    CloseLogFile();
}

void KbdLogs::LogEvent(const char *Format, ...)
{
    va_list ArgList;
    int PrefixSize = 0;
    char szOutString[2048];

    SYSTEMTIME stTime;
    GetLocalTime(&stTime);

    PrefixSize = sprintf_s(szOutString, sizeof(szOutString),
                           "%02d/%02d/%02d %02d:%02d:%02d.%03d - %d.%d>: ", stTime.wYear,
                           stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute,
                           stTime.wSecond, stTime.wMilliseconds, GetCurrentProcessId(),
                           GetCurrentThreadId());

    va_start(ArgList, Format);

    if (_vsnprintf(&szOutString[PrefixSize], sizeof(szOutString) - PrefixSize, Format,
                   ArgList) < 0)
    {
        OutputDebugStringA("Could not pack string into 2048 bytes\n");
    }
    else
    {
        OutputDebugStringA(szOutString);
        // OutputDebugStringA("\n");
    }

    if (currentLogFile)
    {
        fprintf(currentLogFile, "%s\n", szOutString);
        fflush(currentLogFile);
    }
    va_end(ArgList);
}

void KbdLogs::OpenLogFile(const char *pszLogFile)
{
    LogEvent("Starting logging in file {%s}...", pszLogFile);

    currentLogFile = fopen(pszLogFile, "a");
    if (!currentLogFile)
    {
        LogEvent("Could not open file for append!");
    }
}

void KbdLogs::CloseLogFile()
{
    if (currentLogFile)
    {
        fclose(currentLogFile);
        currentLogFile = NULL;
    }
}
}
