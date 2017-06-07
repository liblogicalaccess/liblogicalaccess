/**
 * \file settings.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Settings header
 */

#ifndef LOGICALACCESS_SETTINGS_HPP
#define LOGICALACCESS_SETTINGS_HPP

#include <string>
#include <sstream>
#include <vector>
#ifndef LIBLOGICALACCESS_API
#ifdef _MSC_VER
#include "msliblogicalaccess.h"
#else
#define LIBLOGICALACCESS_API
#endif
#endif

#ifdef _MSC_VER
#include <Windows.h>
extern HMODULE __hLibLogicalAccessModule;
#endif

namespace logicalaccess
{
    class LIBLOGICALACCESS_API Settings
    {
    public:

        static Settings* getInstance();

        void Initialize();
        static void Uninitialize();

        /* Logs */
        bool IsLogEnabled;
        std::string LogFileName;
        bool LogToStderr;
        bool SeeWaitInsertionLog;
        bool SeeWaitRemovalLog;
        bool SeeCommunicationLog;
        bool SeePluginLog;
        bool ColorizeLog;
        bool ContextLog;

        /* Auto-Detection */
        bool IsAutoDetectEnabled;
        long int AutoDetectionTimeout;

        /* Serial port configuration */
        bool IsConfigurationRetryEnabled;
        long int ConfigurationRetryTimeout;

        /* Default Reader */
        std::string DefaultReader;
        std::vector<std::string> PluginFolders;

        /* Networking */

        /**
         * The default Data Transport timeout to use.
         *
         * If not specified, use 3000.
         */
        int DataTransportTimeout;

        static std::string getDllPath();

    protected:

        Settings();

        void LoadSettings();
        void SaveSettings();

        static Settings* instance;

    private:

        void reset();
    };
}

#endif