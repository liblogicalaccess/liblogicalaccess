// islogkbdhook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>
#include <iostream>
#include <boost\program_options.hpp>

#include "Resource.h"
#include "islogkbdhooklib.hpp"
#include "islogkbdlogs.hpp"
#include "islogkbdsettings.hpp"
#include "../keyboardsharedstruct.hpp"
#include "inputdevice.hpp"

#define MAX_LOADSTRING 100

// Global Variables:
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND _hwnd;

UINT hwnd = 0;
UINT msgId = 0;
UINT msgId_LL = 0;

std::string sharedname;
std::string klayout;

HANDLE shKeyboard;
logicalaccess::KeyboardSharedStruct* sKeyboard;
HANDLE hKbdEvent;
HANDLE hKbdEventProcessed;
HANDLE hHostEvent;
HANDLE hStillAliveEvent;

HANDLE hCheckThrd;
bool continueHostCheck;

HHOOK hKbdHook_ll;
HHOOK hKbdHook;
bool skipKey;

std::vector<logicalaccess::KeyboardEntry> hidDevices;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
long				LoadKbdFileMapping();
void				FreeKbdFileMapping();
long				LoadKbdEvent();
void				FreeKbdEvent();
void				HookRawInput(HWND hwnd);
bool				ProcessRawInputMessage(HRAWINPUT rawInputHeader);
DWORD WINAPI		CheckThread(LPVOID lpThreadParameter);

/**
 * \brief The application entry point.
 * \param argc The arguments count.
 * \param argv The arguments.
 */
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	OutputDebugStringA("#islogkbdhook::_tWinMain# started.");

	islogkbdlib::KbdSettings::getInstance()->Initialize();
	islogkbdlib::KbdLogs::getInstance()->Initialize();

	islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# begins.");

	unsigned int wid = 0;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "get help information")
		("own,o", "hook and display in his own Window")
		("window,w", boost::program_options::value<unsigned int>(&wid), "hook and transfer messages to a Window handle")
		("msgid,m", boost::program_options::value<unsigned int>(&msgId), "message id for the hook to transfer to the Window")
		("msgidll,l", boost::program_options::value<unsigned int>(&msgId_LL), "low level message id for the hook to transfer to the Window")
		("shared,s", boost::program_options::value<std::string>(&sharedname), "hook and communicate with a controller through shared memory")
	;

	std::vector<std::string> args = boost::program_options::split_winmain(lpCmdLine);
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(args).options(desc).run(), vm);
	boost::program_options::notify(vm);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HOOK, szWindowClass, MAX_LOADSTRING);
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Registering class...");
	MyRegisterClass(hInstance);

	// Perform application initialization:
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Initializing instance...");
	if (!InitInstance (hInstance, (vm.count("own") || vm.count("help")) ? SW_SHOW : SW_HIDE))
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Error. Unable to initialize instance!");
		return FALSE;
	}

	if (vm.count("window"))
	{
		hwnd = wid;
	}
	else
	{
		hwnd = (UINT)_hwnd;
		msgId = 0x401;
		msgId_LL = 0x402;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOOK));

	//HWND hwnd = FindWindow(NULL, L"Keyboard Redirector");
	if (hwnd == 1)
		hwnd = (UINT)FindWindow(NULL, "Keyboard Redirector");
	if (hwnd == 0)
		return -3; // Invalid window handle;

	if (msgId != 0)
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Setting hook...");
		SetHook((HWND)hwnd, msgId);
	}

	if (msgId_LL != 0)
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Setting Low Level hook...");
		SetHook_LL((HWND)hwnd, msgId_LL);
	}

	if (vm.count("shared"))
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# \"shared\" command line parameter detected!");

		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Loading file mapping...");
		LoadKbdFileMapping();
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Loading keyboard events...");
		LoadKbdEvent();
		continueHostCheck = true;

		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Starting check thread...");
		hCheckThrd = CreateThread(NULL, 0, CheckThread, NULL, 0, NULL);

		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Getting devices...");
		hidDevices = KBDHOOK::InputDevice::getDeviceList();
		int i = 0;
		for (std::vector<logicalaccess::KeyboardEntry>::iterator it = hidDevices.begin(); it != hidDevices.end(); ++it, ++i)
		{
			if (i < MAX_KEYBOARD_DEVICES)
			{
				sKeyboard->devices[i] = *it;
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# sKeyboard->devices[%d].name = {%s}", i, sKeyboard->devices[i].name);
			}
			else
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# To much keyboards... Stopping at limit {%d}.", MAX_KEYBOARD_DEVICES);
				break;
			}
		}

		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Setting host event...");
		SetEvent(hHostEvent);

		skipKey = false;
		HookRawInput((HWND)hwnd);
	}
	else
	{
		if (vm.count("help"))
		{
			SendMessage(_hwnd, WM_COMMAND, IDC_SHOWHELP, 0);
		}
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Need to shutdown.");

	if (vm.count("shared"))
	{
		//UnhookWindowsHookEx(hKbdHook_ll);
		sharedname = "";

		continueHostCheck = false;
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Releasing keyboard events...");
		FreeKbdEvent();
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Releasing file mapping...");
		FreeKbdFileMapping();
	}
	
	if (msgId != 0)
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Clearing hook...");
		ClearHook((HWND)hwnd);
	}

	if (msgId_LL != 0)
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#_tWinMain# Clearing Low Level hook...");
		ClearHook_LL((HWND)hwnd);
	}

	islogkbdlib::KbdLogs::getInstance()->Uninitialize();
	islogkbdlib::KbdSettings::getInstance()->Uninitialize(); 

	OutputDebugStringA("#islogkbdhook::_tWinMain# done.");

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOOK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HOOK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= 0;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
	  islogkbdlib::KbdLogs::getInstance()->LogEvent("#InitInstance# Unable to create window {%d}.", GetLastError());
	  return FALSE;
   }
   _hwnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# WM_COMMAND received.");
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
			case IDC_SHOWHELP:
				{
					HDC hdc = GetDC(hWnd);
					TextOut(hdc, 30, 30, "Help !", 100);
				}
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case 0x401:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# 0x401 received.");
			HDC hdc = GetDC(hWnd);
			LPSTR str = new CHAR[100];
			memset(str, 0, 100 * sizeof(CHAR));
			wsprintf(str, "msghook: wparam=%#08x %#08x", wParam, lParam);
			TextOut(hdc, 30, 30, str, 100);
			ReleaseDC(hWnd, hdc);

			if (skipKey)
			{
				return -1;
			}
		}
		break;
	case 0x402:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# 0x402 received.");
			HDC hdc = GetDC(hWnd);
			LPSTR str = new CHAR[100];
			memset(str, 0, 100 * sizeof(CHAR));
			wsprintf(str, "msghook LL: wparam=%#08x %#08x", wParam, lParam);
			TextOut(hdc, 30, 80, str, 100);
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_PAINT:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# WM_PAINT received.");
			hdc = BeginPaint(hWnd, &ps);

			LPSTR str = new CHAR[100];
			memset(str, 0, 100 * sizeof(CHAR));
			wsprintf(str, "hwnd: %d msgId=%#x msgId_LL %#x", hwnd, msgId, msgId_LL);
			TextOut(hdc, 10, 10, str, 100);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_INPUT:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# WM_INPUT received.");
			if (ProcessRawInputMessage((HRAWINPUT)lParam))
			{
				// Could have race condition if the user type on the real keyboard on the same time.
				skipKey = true;
				//MSG newmsg;
				//PeekMessage(&newmsg, 0, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE);
				//PeekMessage(&newmsg, 0, WM_INPUT, WM_INPUT, PM_REMOVE);
			}
			else
			{
				skipKey = false;
			}
		}
		break;
	case WM_DESTROY:
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#WndProc# WM_INPUT received.");
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

long LoadKbdFileMapping()
{
	long ret = 0;
	char sharedFullname[512];
	memset(sharedFullname, 0x00, sizeof(sharedFullname));
	sprintf_s(sharedFullname, sizeof(sharedFullname), "%s%s", KEYBOARD_SHAREDDATA, sharedname.c_str());
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Open file mapping with name {%s}...", sharedFullname);

	shKeyboard = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, sharedFullname);
	if (!shKeyboard)
	{
		ret = GetLastError();
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Error. Unable to open file mapping {%d}.", ret);
	}
	else
	{
		sKeyboard = (logicalaccess::KeyboardSharedStruct*)MapViewOfFile(shKeyboard, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!sKeyboard)
		{
			ret = GetLastError();
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Error. Unable to map file {%d}.", ret);
		}
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Keyboard structure size {%d}.", sizeof(sKeyboard));
	}
	
	return ret;
}

void FreeKbdFileMapping()
{
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#FreeKbdFileMapping# begins.");
	if (sKeyboard != NULL)
	{
		UnmapViewOfFile(sKeyboard);
		sKeyboard = NULL;
	}

	if (shKeyboard != NULL)
	{
		CloseHandle(shKeyboard);
		shKeyboard = NULL;
	}
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#FreeKbdFileMapping# done.");
}

long LoadKbdEvent()
{
	long ret = 0;

	char eventName[512];
	memset(eventName, 0x00, sizeof(eventName));
	sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENT, sharedname.c_str());
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Opening event name {%s}...", eventName);

	hKbdEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
	if (!hKbdEvent)
	{
		ret = GetLastError();
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdEvent# Error. Unable to open event {%s} {%d}.", eventName, ret);
	}
	else
	{
		memset(eventName, 0x00, sizeof(eventName));
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENTPROCESEED, sharedname.c_str());
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Opening event name {%s}...", eventName);

		hKbdEventProcessed = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
		if (!hKbdEventProcessed)
		{
			ret = GetLastError();
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdEvent# Error. Unable to open event {%s} {%d}.", eventName, ret);
		}
		else
		{
			memset(eventName, 0x00, sizeof(eventName));
			sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_HOSTEVENT, sharedname.c_str());
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Opening event name {%s}...", eventName);

			hHostEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
			if (!hHostEvent)
			{
				ret = GetLastError();
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdEvent# Error. Unable to open event {%s} {%d}.", eventName, ret);
			}
			else
			{
				memset(eventName, 0x00, sizeof(eventName));
				sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_STILLALIVEEVENT, sharedname.c_str());
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdFileMapping# Opening event name {%s}...", eventName);

				hStillAliveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
				if (!hStillAliveEvent)
				{
					ret = GetLastError();
					islogkbdlib::KbdLogs::getInstance()->LogEvent("#LoadKbdEvent# Error. Unable to open event {%s} {%d}.", eventName, ret);
				}
			}
		}
	}

	return ret;
}

void FreeKbdEvent()
{
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#FreeKbdEvent# begins.");
	if (hKbdEvent != NULL)
	{
		CloseHandle(hKbdEvent);
		hKbdEvent = NULL;
	}

	if (hKbdEventProcessed != NULL)
	{
		CloseHandle(hKbdEventProcessed);
		hKbdEventProcessed = NULL;
	}

	if (hHostEvent != NULL)
	{
		CloseHandle(hHostEvent);
		hHostEvent = NULL;
	}

	if (hStillAliveEvent != NULL)
	{
		CloseHandle(hStillAliveEvent);
		hStillAliveEvent = NULL;
	}
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#FreeKbdEvent# done.");
}

void HookRawInput(HWND hwnd)
{
	RAWINPUTDEVICE rid[1];
 
	rid[0].usUsagePage = 0x01;      // USB HID Generic Desktop Page
	rid[0].usUsage = 0x06;          // Keyboard Usage ID
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = hwnd;
 
	RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE));
}

bool ProcessRawInputMessage(HRAWINPUT rawInputHeader)
{
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# started.");
	bool handled = false;
	unsigned int size = 0;
 
	// First we call GetRawInputData() to set the value of size, which
	// we will the nuse to allocate the appropriate amount of memory in the buffer.
	if (GetRawInputData( rawInputHeader, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) == 0)
	{
		RAWINPUT raw;
 
		if (GetRawInputData( rawInputHeader, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == size)
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Raw input data retrieved successfully! Checking keyboard device...");

			islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Selected device {%s}", sKeyboard->selectedDeviceName);
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# - VS - ");
			// Device filtering
			for (std::vector<logicalaccess::KeyboardEntry>::iterator it = hidDevices.begin(); it != hidDevices.end() && !handled; ++it)
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Device {%s}", it->name);

				if (std::string(sKeyboard->selectedDeviceName) == std::string(it->name))
				{
					islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Device found! Checking if same handle...");
					handled = (raw.header.hDevice == (HANDLE)it->handle);
				}
			}
 
			if (handled)
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Raw is coming from the selected keyboard device!");

				if (raw.header.dwType == RIM_TYPEKEYBOARD)
				{
					if (raw.data.keyboard.Message == WM_KEYDOWN)
					{
						BYTE kstate[256];
 
						if (GetKeyboardState(kstate) == TRUE)
						{
							if (!sharedname.empty())
							{
								HKL useLayout = NULL;

								if (!std::string(sKeyboard->keyboardLayout).empty())
								{
									islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Need to use the configured layout {%s}.", sKeyboard->keyboardLayout);

									char currentLayout[KL_NAMELENGTH];
									memset(currentLayout, 0x00, sizeof(currentLayout));

									if (GetKeyboardLayoutName(currentLayout) == TRUE)
									{
										if (strcmp(currentLayout, sKeyboard->keyboardLayout) != 0)
										{
											useLayout = LoadKeyboardLayout(sKeyboard->keyboardLayout, KLF_SUBSTITUTE_OK);
										}
									}
								}

								char convertedkey[2];
								memset(convertedkey, 0x00, sizeof(convertedkey));

								if (raw.data.keyboard.VKey != VK_RETURN)
								{
									if (ToAsciiEx(raw.data.keyboard.VKey, raw.data.keyboard.MakeCode, kstate, (LPWORD)convertedkey, 0, useLayout) == 1)
									{
										sKeyboard->enteredKeyChar = convertedkey[0];
										islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Final key char {%c}. Setting event..", convertedkey[0]);

										if (SetEvent(hKbdEvent) == TRUE)
										{
											unsigned long timeout = 1000;
											islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Waiting for event \"hKbdEventProcessed\" to be set by host during timeout {%u}...", timeout);
											
											if (WaitForSingleObject(hKbdEventProcessed, timeout) == WAIT_TIMEOUT)
											{
												islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Timeout reached! Host has not processed the input message...");
											}
											else
											{
												islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Event successfully processed by host...");
											}
											ResetEvent(hKbdEventProcessed);
										}
										else
										{
											islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Unable to set event. Error {%d}", GetLastError());
										}
									}
									else
									{
										islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Error ToAsciiEx {%d}", GetLastError());
									}
								}
								else
								{
									islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# VK_RETURN detected. Ignoring...");
								}
							}
							else
							{
								islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Shared name is empty. Ignoring...");
							}
						}
						else
						{
							islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Error GetKeyboardState {%d}", GetLastError());
						}
					}
					else
					{
						islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# raw.data.keyboard.Message != WM_KEYDOWN. dwType {%u}", raw.data.keyboard.Message);
					}
				}
				else
				{
					islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# raw.header.dwType != RIM_TYPEKEYBOARD. dwType {%ul}", raw.header.dwType);
				}
			}
			else
			{
				islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Keyboard device is not handled. Ignoring...");
			}
		}
		else
		{
			islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Error GetRawInputData {%d}", GetLastError());
		}
	}
	else
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# Error GetRawInputData {%d}", GetLastError());
	}
 
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#ProcessRawInputMessage# returns handled? {%d}", handled);
	return handled;
}

DWORD WINAPI CheckThread(LPVOID lpThreadParameter)
{
	unsigned long timeout = 2000; 
	islogkbdlib::KbdLogs::getInstance()->LogEvent("#CheckThread# begins. Checking every {%u} milliseconds if host is present...", timeout);

	while (continueHostCheck && WaitForSingleObject(hStillAliveEvent, timeout) == WAIT_OBJECT_0)
	{
		//islogkbdlib::KbdLogs::getInstance()->LogEvent("#CheckThread# Host still present!");
		ResetEvent(hStillAliveEvent);
	}

	if (continueHostCheck)
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#CheckThread# Unable to contact host. Exiting...");
	}
	else
	{
		islogkbdlib::KbdLogs::getInstance()->LogEvent("#CheckThread# Thread exiting requested. Exiting...");
	}

	ExitProcess(0);
}
