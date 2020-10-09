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
#include "logicalaccess/plugins/llacommon/lla_common_api.hpp"

namespace logicalaccess
{
class LLA_COMMON_API Settings
{
  public:
    static Settings *getInstance();

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

    /**
     * EV2 Proximity Check timer.
     * EV2 Chip send a "expected response time" that let us known
     * how long the getting a response from card should take.
     *
     * However, in PCSC mode we cannot time this, because we can
     * only time full command/response pair.
     * By default this multiplier is set to 2.
     */
    double ProximityCheckResponseTimeMultiplier;

    static std::string getDllPath();

  protected:
    Settings();

    void LoadSettings();
    void SaveSettings() const;

    static Settings *instance;

  private:
    void reset();
};
}

#endif