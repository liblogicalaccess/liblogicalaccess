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

	KbdSettings::KbdSettings() : IsLogEnabled(false)
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
            char tmp[128];
			DWORD error = ERROR_SUCCESS;
			if (!GetModuleFileNameA((HMODULE)&__ImageBase, szAppPath, sizeof(szAppPath)-1))
			{
				error = GetLastError();
				sprintf(tmp, "Cannot get module file name. Last error code: %lu. Trying with GetModuleHandle first...", error);
				OutputDebugStringA(tmp);
				HMODULE hm = NULL;
				if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCSTR) &instance, 
                            &hm))
				{
					int ret = GetLastError();
					sprintf(tmp, "GetModuleHandle returned %d\n", ret);
					OutputDebugStringA(tmp);
				}				

				if (!GetModuleFileNameA(hm, szAppPath, sizeof(szAppPath)-1))
				{
					error = GetLastError();
					sprintf(tmp, "Cannot get module file name. Last error code: %d. Trying with hmodule (%lu) from dllmain...", error, __hKbdHookModule);
					OutputDebugStringA(tmp);
					if (__hKbdHookModule == NULL)
					{
						sprintf(tmp, "hmodule from dllmain is null.");
						OutputDebugStringA(tmp);
					
					}
					else
					{
						if (!GetModuleFileNameA(__hKbdHookModule, szAppPath, sizeof(szAppPath)-1))
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
#else
		return boost::filesystem::current_path().string();
#endif
	}
}