// islogkbdhook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>
#include <iostream>
#include <boost\program_options.hpp>

#include "Resource.h"
#include "islogkbdhooklib.h"
#include "../KeyboardSharedStruct.h"
#include "InputDevice.h"


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
LOGICALACCESS::KeyboardSharedStruct* sKeyboard;
HANDLE hKbdEvent;
HANDLE hKbdEventProcessed;
HANDLE hHostEvent;
HANDLE hStillAliveEvent;

HANDLE hCheckThrd;
bool continueHostCheck;

HHOOK hKbdHook_ll;
HHOOK hKbdHook;
bool skipKey;

std::vector<LOGICALACCESS::KeyboardEntry> hidDevices;

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
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, (vm.count("own") || vm.count("help")) ? SW_SHOW : SW_HIDE))
	{
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
		SetHook((HWND)hwnd, msgId);

	if (msgId_LL != 0)
		SetHook_LL((HWND)hwnd, msgId_LL);

	if (vm.count("shared"))
	{
		LoadKbdFileMapping();
		LoadKbdEvent();
		continueHostCheck = true;
		hCheckThrd = CreateThread(NULL, 0, CheckThread, NULL, 0, NULL);

		hidDevices = KBDHOOK::InputDevice::getDeviceList();
		int i = 0;
		for (std::vector<LOGICALACCESS::KeyboardEntry>::iterator it = hidDevices.begin(); it != hidDevices.end(); ++it, ++i)
		{
			if (i < MAX_KEYBOARD_DEVICES)
			{
				sKeyboard->devices[i] = *it;
			}
		}
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

	if (vm.count("shared"))
	{
		//UnhookWindowsHookEx(hKbdHook_ll);
		sharedname = "";

		continueHostCheck = false;
		FreeKbdEvent();
		FreeKbdFileMapping();
	}
	
	if (msgId != 0)
		ClearHook((HWND)hwnd);

	if (msgId_LL != 0)
		ClearHook_LL((HWND)hwnd);

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
		break;
	case 0x401:
		{
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
		PostQuitMessage(0);
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

	shKeyboard = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, sharedFullname);
	if (!shKeyboard)
	{
		ret = GetLastError();
	}
	else
	{
		sKeyboard = (LOGICALACCESS::KeyboardSharedStruct*)MapViewOfFile(shKeyboard, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!sKeyboard)
		{
			ret = GetLastError();
		}
	}
	
	return ret;
}

void FreeKbdFileMapping()
{
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
}

long LoadKbdEvent()
{
	long ret = 0;

	char eventName[512];
	memset(eventName, 0x00, sizeof(eventName));
	sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENT, sharedname.c_str());

	hKbdEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
	if (!hKbdEvent)
	{
		ret = GetLastError();
	}
	else
	{
		memset(eventName, 0x00, sizeof(eventName));
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENTPROCESEED, sharedname.c_str());

		hKbdEventProcessed = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
		if (!hKbdEventProcessed)
		{
			ret = GetLastError();
		}
		else
		{
			memset(eventName, 0x00, sizeof(eventName));
			sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_HOSTEVENT, sharedname.c_str());

			hHostEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
			if (!hHostEvent)
			{
				ret = GetLastError();
			}
			else
			{
				memset(eventName, 0x00, sizeof(eventName));
				sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_STILLALIVEEVENT, sharedname.c_str());

				hStillAliveEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
				if (!hStillAliveEvent)
				{
					ret = GetLastError();
				}
			}
		}
	}

	return ret;
}

void FreeKbdEvent()
{
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
	bool handled = false;
	unsigned int size = 0;
 
	// First we call GetRawInputData() to set the value of size, which
	// we will the nuse to allocate the appropriate amount of memory in
	// the buffer.
	if (GetRawInputData(
			rawInputHeader,
			RID_INPUT,
			NULL,
			&size,
			sizeof(RAWINPUTHEADER)) == 0)
	{
		RAWINPUT raw;
 
		if (GetRawInputData(
				rawInputHeader,
				RID_INPUT,
				&raw,
				&size,
				sizeof(RAWINPUTHEADER)) == size)
		{ 
			// Device filtering
			for (std::vector<LOGICALACCESS::KeyboardEntry>::iterator it = hidDevices.begin(); it != hidDevices.end() && !handled; ++it)
			{
				if (std::string(sKeyboard->selectedDeviceName) == std::string(it->name))
				{
					handled = (raw.header.hDevice == it->handle);
				}
			}
 
			if (handled)
			{
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
										if (SetEvent(hKbdEvent) == TRUE)
										{
											WaitForSingleObject(hKbdEventProcessed, 10000);
											ResetEvent(hKbdEventProcessed);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
 
	return handled;
}

DWORD WINAPI CheckThread(LPVOID lpThreadParameter)
{
	while (continueHostCheck && WaitForSingleObject(hStillAliveEvent, 10000) == WAIT_OBJECT_0)
	{
		ResetEvent(hStillAliveEvent);
	}

	ExitProcess(0);
}