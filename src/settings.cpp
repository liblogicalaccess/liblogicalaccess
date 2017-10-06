/**
 * \file settings.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Base class settings informations.
 */

#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "logicalaccess/settings.hpp"
#include "logicalaccess/logs.hpp"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef _MSC_VER
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

namespace logicalaccess
{
    Settings* Settings::instance = NULL;

    Settings::Settings()
    {
        reset();
    }

    void Settings::Initialize()
    {
        static bool alreadyInit = false;

        if (alreadyInit == true)
            return;
        alreadyInit = true;

        try
        {
            LoadSettings();

            LOG(LogLevel::INFOS) << "Log [enabled " << IsLogEnabled << " filename " << LogFileName << " seewaitinsertion " << SeeWaitInsertionLog << " seewaitremoval " << SeeWaitRemovalLog << "]";
            LOG(LogLevel::INFOS) << "Auto-detection [enabled " << IsAutoDetectEnabled << " timeout " << AutoDetectionTimeout << "]";
            LOG(LogLevel::INFOS) << "Retry serial port configuration [enabled " << IsConfigurationRetryEnabled << " timeout " << ConfigurationRetryTimeout << "]";

            if (IsLogEnabled && !Logs::logfile.is_open())
            {
#ifdef _MSC_VER
                Logs::logfile.open((getDllPath() + "/" + LogFileName), std::ios::out | std::ios::app);
#else
                Logs::logToStderr = LogToStderr;
                Logs::logfile.open(LogFileName, std::ios::out | std::ios::app);
#endif
            }
        }
        catch (...) { reset(); }
    }

    void Settings::Uninitialize()
    {
        if (Logs::logfile)
        {
            Logs::logfile.close();
        }
    }

    Settings* Settings::getInstance()
    {
        if (instance == NULL)
        {
            instance = new Settings();
            LOG(LogLevel::INFOS) << "New settings instance created.";
        }
        instance->Initialize();
        return instance;
    }

    // Loads log settings structure from the specified XML file
    void Settings::LoadSettings()
    {
        try
        {
            reset();

            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;

            // Load the XML file into the property tree. If reading fails
            // (cannot open file, parse error), an exception is thrown.
            std::list<std::string> configfolder;
            configfolder.push_back(getDllPath() + "/liblogicalaccess.config");
#if defined(__unix__) && !defined(ANDROID)
            configfolder.push_back("/etc/liblogicalaccess.config");
#endif
            for (std::list<std::string>::iterator it = configfolder.begin(); it != configfolder.end(); ++it)
            {
                try
                {
                    if (boost::filesystem::exists(*it)) {
                        read_xml(*it, pt);
                        break;
                    }
                }
                catch (boost::property_tree::xml_parser::xml_parser_error &e)
                {
                    if (e.message() == "cannot open file")
                        continue;
                    throw;
                }
            }

            IsLogEnabled = pt.get("config.log.enabled", false);
            LogFileName = pt.get<std::string>("config.log.filename", "liblogicalaccess.log");
            LogToStderr = pt.get<bool>("config.log.to_stderr", false);
            SeeWaitInsertionLog = pt.get("config.log.seewaitinsertion", false);
            SeeWaitRemovalLog = pt.get("config.log.seewaitremoval", false);
            SeeCommunicationLog = pt.get("config.log.seecommunication", false);
            SeePluginLog = pt.get("config.log.seeplugin", false);
            ColorizeLog = pt.get("config.log.colorize", false);
            ContextLog = pt.get("config.log.context", false);

            IsAutoDetectEnabled = pt.get("config.autodetect.enabled", false);
            AutoDetectionTimeout = pt.get<long int>("config.autodetect.timeout", 400);

            IsConfigurationRetryEnabled = pt.get("config.retrySerialConfiguration.enabled", false);
            ConfigurationRetryTimeout = pt.get<long int>("config.retrySerialConfiguration.timeout", 500);

            DefaultReader = pt.get<std::string>("config.reader.default", "PCSC");

            DataTransportTimeout = pt.get<int>("config.dataTransportTimeout", 3000);

            PluginFolders.clear();
            BOOST_FOREACH(ptree::value_type const& v, pt.get_child("config.PluginFolders"))
            {
                if (v.first == "Folder")
                {
                    std::string folder = v.second.get<std::string>("");
                    const std::string currentName = "$current";
                    size_t start_pos = folder.find(currentName);
                    if(start_pos != std::string::npos)
                    {
                        folder.replace(start_pos, currentName.length(), getDllPath());
                    }

                    LOG(LogLevel::PLUGINS) << "Library folder: " << folder;
                    PluginFolders.push_back(folder);
                }
            }
        }
        catch (...)
        {
            PluginFolders.clear();
        }

        if (PluginFolders.size() == 0)
            PluginFolders.push_back(getDllPath());
    }

    void Settings::SaveSettings()
    {
        try
        {
            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;

            // Put log filename in property tree
            pt.put("config.log.enabled", IsLogEnabled);
            pt.put("config.log.filename", LogFileName);
            pt.put("config.log.to_stderr", LogToStderr);
            pt.put("config.log.seewaitinsertion", SeeWaitInsertionLog);
            pt.put("config.log.seewaitremoval", SeeWaitRemovalLog);
            pt.put("config.log.seecommunication", SeeCommunicationLog);
            pt.put("config.log.seeplugin", SeePluginLog);
            pt.put("config.log.colorize", ColorizeLog);
            pt.put("config.log.context", ContextLog);

            pt.put("config.autodetect.enabled", IsAutoDetectEnabled);
            pt.put("config.autodetect.timeout", AutoDetectionTimeout);

            pt.put("config.retrySerialConfiguration.enabled", IsConfigurationRetryEnabled);
            pt.put("config.retrySerialConfiguration.timeout", ConfigurationRetryTimeout);

            pt.put("config.reader.default", "PCSC");

            pt.put("config.dataTransportTimeout", DataTransportTimeout);

            // Write the property tree to the XML file.
            write_xml((getDllPath() + "/liblogicalaccess.config"), pt);
        }
        catch (...) {}
    }

    void Settings::reset()
    {
        IsLogEnabled = false;
        LogFileName = "liblogicalaccess.log";
        LogToStderr = false;
        SeeWaitInsertionLog = false;
        SeeWaitRemovalLog = false;
        SeeCommunicationLog = false;
        SeePluginLog = false;
        ColorizeLog = false;
        ContextLog = false;

        IsAutoDetectEnabled = false;
        AutoDetectionTimeout = 400;

        IsConfigurationRetryEnabled = false;
        ConfigurationRetryTimeout = 500;
        DefaultReader = "PCSC";
        PluginFolders.clear();
        PluginFolders.push_back(getDllPath());

        DataTransportTimeout = 3000;
    }

    std::string Settings::getDllPath()
    {
#ifdef _MSC_VER
        char szAppPath[MAX_PATH];
        memset(szAppPath, 0x00, sizeof(szAppPath));
        static std::string path = ".";

        if (path == ".")
        {
            char tmp[128];
            DWORD error = ERROR_SUCCESS;
            if (!GetModuleFileNameA((HMODULE)&__ImageBase, szAppPath, sizeof(szAppPath) - 1))
            {
                error = GetLastError();
                sprintf(tmp, "Cannot get module file name. Last error code: %lu. Trying with GetModuleHandle first...", error);
                OutputDebugStringA(tmp);
                HMODULE hm = NULL;
                if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    (LPCSTR)&instance,
                    &hm))
                {
                    int ret = GetLastError();
                    sprintf(tmp, "GetModuleHandle returned %d\n", ret);
                    OutputDebugStringA(tmp);
                }

                if (!GetModuleFileNameA(hm, szAppPath, sizeof(szAppPath) - 1))
                {
                    error = GetLastError();
                    sprintf(tmp, "Cannot get module file name. Last error code: %lu. Trying with hmodule (%p) from dllmain...", error, __hLibLogicalAccessModule);
                    OutputDebugStringA(tmp);
                    if (__hLibLogicalAccessModule == NULL)
                    {
                        sprintf(tmp, "hmodule from dllmain is null.");
                        OutputDebugStringA(tmp);
                    }
                    else
                    {
                        if (!GetModuleFileNameA(__hLibLogicalAccessModule, szAppPath, sizeof(szAppPath) - 1))
                        {
                            error = GetLastError();
                        }
                        else
                        {
                            error = ERROR_SUCCESS;
                        }
                    }
                }
                else
                {
                    error = ERROR_SUCCESS;
                }
            }

            if (error == ERROR_SUCCESS)
            {
                std::string tmp(szAppPath);
                size_t index = tmp.find_last_of("/\\");
                if (index != std::string::npos)
                {
                    tmp = tmp.substr(0, index);
                }
                path = tmp;
            }
            else
            {
                sprintf(tmp, "Still cannot get module file name. Last error code: %lu.", error);
                OutputDebugStringA(tmp);
            }

            sprintf(tmp, "Current dll path is: %s.", path.c_str());
            OutputDebugStringA(tmp);
        }

        return path;
#elif defined(__APPLE__)
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        char path[PATH_MAX];
        if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        {
            return boost::filesystem::current_path().string();
        }
        CFRelease(resourcesURL);

        return std::string(path, strlen(path));
#else
        return boost::filesystem::current_path().string();
#endif
    }
}
