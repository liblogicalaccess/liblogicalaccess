#include "stdafx.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "islogkbdsettings.hpp"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string>

#ifdef _MSC_VER
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

namespace islogkbdlib
{
	KbdSettings* KbdSettings::instance = NULL;

	KbdSettings::KbdSettings()
	{
	}
	
	void KbdSettings::Initialize()
	{
		static bool alreadyInit = false;

		if (alreadyInit == true)
			return;
		alreadyInit = true;

		try
		{
			OutputDebugStringA("#KbdSettings::Initialize# begin.");
			LoadSettings();
		}
		catch(...) { reset(); }
	}
	
	void KbdSettings::Uninitialize()  
	{
	}

	KbdSettings* KbdSettings::getInstance()
	{
		if (instance == NULL)
		{
			instance = new KbdSettings();
			instance->Initialize();
		}
		return instance;
	}

	void KbdSettings::LoadSettings()
	{
		try
		{
			reset();

			using boost::property_tree::ptree;
			ptree pt;

			read_xml((getDllPath() + "/islogkbdsettings.config"), pt);

			IsLogEnabled = pt.get("config.log.enabled", false);
			LogFileName = pt.get<std::string>("config.log.filename", "islogkbdlogs.log");

			if (IsLogEnabled)
			{
				OutputDebugStringA("#KbdSettings::LoadSettings# Log is enabled on file:");
				OutputDebugStringA(LogFileName.c_str());
			}
		}
		catch (...) 
		{
		}
	}

	void KbdSettings::SaveSettings()
	{
		try
		{
			using boost::property_tree::ptree;
			ptree pt;

			pt.put("config.log.enabled", IsLogEnabled);
			pt.put("config.log.filename", LogFileName);

			write_xml((getDllPath() + "\\islogkbdsettings.config"), pt);
		}
		catch (...) { }
	}
	
	void KbdSettings::reset()  
	{  
		IsLogEnabled = false;  
		LogFileName = "islogkbdlogs.log";
	}

	std::string KbdSettings::getDllPath()
	{
#ifdef _MSC_VER
		char szAppPath[MAX_PATH];
		memset(szAppPath, 0x00, sizeof(szAppPath));
		static std::string path = ".";

		if (path == ".")
		{
			DWORD size = GetModuleFileNameA((HMODULE)&__ImageBase, szAppPath, sizeof(szAppPath)-1);
			DWORD error = GetLastError();
		
			// Return ERROR_FILE_NOT_FOUND on Windows XP with COM layer ever on success... Cannot found any similar issue so ignore it for now...
			if (error != ERROR_SUCCESS && error != ERROR_FILE_NOT_FOUND)
			{
				if (__hKbdHookModule != NULL)
				{
					GetModuleFileNameA(__hKbdHookModule, szAppPath, sizeof(szAppPath)-1);
					error = GetLastError();
				}
			}

			if (error == ERROR_SUCCESS || error == ERROR_FILE_NOT_FOUND)
			{
				std::string tmp(szAppPath);
				size_t index = tmp.find_last_of("/\\");
				if (index != std::string::npos)
				{
					tmp = tmp.substr(0, index);
				}
				path = tmp;
			}
		}

		return path;
#else
		return boost::filesystem::current_path().string();
#endif
	}
}