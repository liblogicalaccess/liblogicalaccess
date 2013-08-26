#include "InputDevice.h"
#include "islogkbdlogs.hpp"

namespace KBDHOOK
{
	std::vector<logicalaccess::KeyboardEntry> InputDevice::getDeviceList()
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# begins.");

		std::vector<logicalaccess::KeyboardEntry> deviceList;

		std::string root1 = "\\\\?\\Root";
		std::string root2 = "\\??\\Root";

		UINT nDevices;
		//PRAWINPUTDEVICELIST pRawInputDeviceList;
		UINT errorCode = GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST));
		if (errorCode == 0 || errorCode == ERROR_INSUFFICIENT_BUFFER)
		{
			std::vector<RAWINPUTDEVICELIST> newlist(nDevices);
			nDevices = GetRawInputDeviceList(&newlist[0], &nDevices, sizeof(RAWINPUTDEVICELIST));
			if (nDevices > 0 )
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# {%u} device(s) detected.", nDevices);

				for (std::vector<RAWINPUTDEVICELIST>::const_iterator it = newlist.begin(); it != newlist.end(); ++it)
				{
					if ((*it).dwType == RIM_TYPEKEYBOARD)
					{
						logicalaccess::KeyboardEntry entry;
						memset(&entry, 0x00, sizeof(entry));
						entry.handle = (unsigned long long) (*it).hDevice;

						UINT nSize = 0;
						GetRawInputDeviceInfo((HANDLE)entry.handle, RIDI_DEVICENAME, NULL, &nSize);

						if (nSize > 0)
						{
							nSize = sizeof(entry.name);
							errorCode = GetRawInputDeviceInfo((HANDLE)entry.handle, RIDI_DEVICENAME, entry.name, &nSize);
						}

						// Skip Terminal Services and Remote Desktop generic keyboards
						if (memcmp(entry.name, root1.c_str(), root1.size()) != 0 && memcmp(entry.name, root2.c_str(), root2.size()) != 0)
						{
							islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# Keyboard device found {%s}!", entry.name);
							deviceList.push_back(entry);
						}
					}
				}
			}
			else
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# No devices detected.");
			}
		}
		else
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# Error GetRawInputDeviceList {%d}!", errorCode);
		}

		islogkbdlib::KbdLogs::getInstance()->LogEvent("#getDeviceList# return.");
		return deviceList;
	}
}