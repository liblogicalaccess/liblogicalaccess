/**
 * \file settings.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Base class settings informations. 
 */

#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "logicalaccess/settings.h"
#include "logicalaccess/logs.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>

#ifdef _MSC_VER
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

namespace LOGICALACCESS
{
	Settings Settings::instance;

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

			if (!Logs::logfile.is_open())
			{
				Logs::logfile.open((getDllPath() + "/" + LogFileName), std::ios::out | std::ios::app);
				
				INFO_("Log [enabled {%d} filename {%s} seewaitinsertion {%d} seewaitremoval {%d}]",
					IsLogEnabled, LogFileName.c_str(), SeeWaitInsertionLog, SeeWaitRemovalLog);
				INFO_("Auto-detection [enabled {%d} timeout {%d}]", IsAutoDetectEnabled, AutoDetectionTimeout);
				INFO_("Retry serial port configuration [enabled {%d} timeout {%d}]", IsConfigurationRetryEnabled, ConfigurationRetryTimeout);
			}
			else
			{
				reset();
			}
		}
		catch(...) { reset(); }
	}
	
	void Settings::Uninitialize()  
	{ 
		if (Logs::logfile)
		{
			Logs::logfile.close();
		}
	}

	Settings& Settings::getInstance()
	{
		instance.Initialize();
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
			read_xml((getDllPath() + "/liblogicalaccess.config"), pt);

			IsLogEnabled = pt.get("config.log.enabled", false);
			LogFileName = pt.get<std::string>("config.log.filename", "liblogicalaccess.log");
			SeeWaitInsertionLog = pt.get("config.log.seewaitinsertion", false);
			SeeWaitRemovalLog = pt.get("config.log.seewaitremoval", false);
			SeeCommunicationLog = pt.get("config.log.seecommunication", false);
			SeePluginLog = pt.get("config.log.seeplugin", false);

			IsAutoDetectEnabled = pt.get("config.autodetect.enabled", false);
			AutoDetectionTimeout = pt.get<long int>("config.autodetect.timeout", 400);

			IsConfigurationRetryEnabled = pt.get("config.retrySerialConfiguration.enabled", false);
			ConfigurationRetryTimeout = pt.get<long int>("config.retrySerialConfiguration.timeout", 500);

			DefaultReader = pt.get<std::string>("config.reader.default", "PCSC");

			PluginFolders.clear();
			BOOST_FOREACH(ptree::value_type const& v, pt.get_child("config.PluginFolders"))
			{
				if (v.first == "Folder")
				{
					std::string folder = v.second.get<std::string>("");
					if (folder == "$current")
						folder = getDllPath();
					PluginFolders.push_back(folder);
				}
			}
			if (PluginFolders.size() == 0)
				PluginFolders.push_back(getDllPath());
		}
		catch (...) { }
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
			pt.put("config.log.seewaitinsertion", SeeWaitInsertionLog);
			pt.put("config.log.seewaitremoval", SeeWaitRemovalLog);
			pt.put("config.log.seecommunication", SeeCommunicationLog);
			pt.put("config.log.seeplugin", SeePluginLog);

			pt.put("config.autodetect.enabled", IsAutoDetectEnabled);
			pt.put("config.autodetect.timeout", AutoDetectionTimeout);

			pt.put("config.retrySerialConfiguration.enabled", IsConfigurationRetryEnabled);
			pt.put("config.retrySerialConfiguration.timeout", ConfigurationRetryTimeout);
   
			pt.put("config.reader.default", "PCSC");


			// Write the property tree to the XML file.
			write_xml((getDllPath() + "\\liblogicalaccess.config"), pt);
		}
		catch (...) { }
	}
	
	void Settings::reset()  
	{  
		IsLogEnabled = false;  
		LogFileName = "liblogicalaccess.log";  
		SeeWaitInsertionLog = false;  
		SeeWaitRemovalLog = false;  
		SeeCommunicationLog = false;
		SeePluginLog = false;
 
		IsAutoDetectEnabled = false;  
		AutoDetectionTimeout = 400;  

		IsConfigurationRetryEnabled = false;  
		ConfigurationRetryTimeout = 500;
		DefaultReader = "PCSC";
		PluginFolders.clear();
		PluginFolders.push_back(getDllPath());
	}

	std::string Settings::getDllPath()
	{
#ifdef _MSC_VER
		std::string path;
	
		char szAppPath[MAX_PATH];
		memset(szAppPath, 0x00, sizeof(szAppPath));

		GetModuleFileNameA((HINSTANCE)&__ImageBase, szAppPath, sizeof(szAppPath)-1);
		if (GetLastError() == ERROR_SUCCESS)
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
			path = ".";
		}

		return path;
#else
		return boost::filesystem::current_path().string();
#endif
	}
}

