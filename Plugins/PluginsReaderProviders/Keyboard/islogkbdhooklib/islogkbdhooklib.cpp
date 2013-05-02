/* 
 *	Copyright (C) 2009 Nate
 *	http://nate.dynalias.net
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
// #define KEYBOARDHOOK_EXPORTS  this is done in the project
#include "islogkbdhooklib.h"

#pragma data_seg(".KEYBOARDHOOK")
HWND s_hWndServer = NULL;
UINT s_message = 0;

HWND s_hWndServer_LL = NULL;
UINT s_message_LL = 0;

#pragma data_seg()
#pragma comment(linker, "/section:.KEYBOARDHOOK,rws")

HINSTANCE hInst;
HHOOK hook;
HHOOK hook_LL;
static LRESULT CALLBACK msghook(UINT nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK msghook_LL(UINT nCode, WPARAM wParam, LPARAM lParam);

#ifdef _MANAGED
#pragma managed(push, off)
#endif


BOOL APIENTRY DllMain( HINSTANCE hInstance,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hInstance;
		break;
	case DLL_PROCESS_DETACH:
		if(s_hWndServer != NULL)
			ClearHook(s_hWndServer);
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

KEYBOARDHOOK_API BOOL SetHook(HWND hWnd, UINT message)
{
	if (s_hWndServer != NULL)
		return FALSE; // already hooked

	hook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)msghook, hInst, 0);
	if (hook == FALSE)
		return FALSE;

	s_hWndServer = hWnd;
	s_message = message;
	return TRUE;
}

KEYBOARDHOOK_API BOOL ClearHook(HWND hWnd)
{
	if ((hWnd == NULL) || (s_hWndServer == NULL) || (hWnd != s_hWndServer))
		return FALSE;

	BOOL unhooked = UnhookWindowsHookEx(hook);
	if (unhooked)
	{
		s_hWndServer = NULL;
		s_message = 0;
	}
	return unhooked;
}

union KeyboardProcLParam
{
	unsigned int lParam;
	struct _keyboardProcLParam {
	   unsigned short repeatCount      : 16;
	   unsigned short scanCode         : 8;
	   unsigned short extendedKey      : 1;
	   unsigned short reserved         : 4;
	   unsigned short contextCode      : 1;
	   unsigned short previousKeyState : 1;
	   unsigned short transitionCode   : 1;
	} values;
};

static LRESULT CALLBACK msghook(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) // The specs say if nCode is < 0 then we just pass straight on.
	{
		CallNextHookEx(hook, nCode, wParam, lParam);
		return 0;
	}

	//LPWSTR str = new WCHAR[100];
	//memset(str, 0, 100);
	//wsprintf(str, L"msghook: nCode=%i wparam=%#08x %#08x\n", nCode, wParam, lParam);
	//OutputDebugString(str);

	WPARAM newWParam = wParam;
	if (nCode == HC_NOREMOVE)
	{
		// I was originally sending HC_NOREMOVE message to my app, but they weren't necessary and were causing problems
		// with debugging in visual studio.
		//newWParam = wParam | 0x80000000;

		CallNextHookEx(hook, nCode, wParam, lParam);
		return 0;
	}

	try
	{
		if (IsWindow(s_hWndServer) == TRUE)
		{
			LRESULT result;
			result = ::SendMessage(s_hWndServer, s_message, newWParam, lParam);
			if (result != 0)
			{
				LPWSTR str = new WCHAR[100];
				memset(str, 0, 100);
				wsprintf(str, L"blocking: nCode=%i wparam=%#08x %#08x\n", nCode, wParam, lParam);
				OutputDebugString(str);
				return -1;
			}
		}
	} catch (...)
	{
	}

	return CallNextHookEx(hook, nCode, wParam, lParam);

}




KEYBOARDHOOK_API BOOL SetHook_LL(HWND hWnd, UINT message)
{
	if (s_hWndServer_LL != NULL)
		return FALSE; // already hooked

	hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)msghook_LL, hInst, 0);
	if (hook == FALSE)
		return FALSE;

	s_hWndServer_LL = hWnd;
	s_message_LL = message;
	return TRUE;
}

KEYBOARDHOOK_API BOOL ClearHook_LL(HWND hWnd)
{
	if ((hWnd == NULL) || (s_hWndServer_LL == NULL) || (hWnd != s_hWndServer_LL))
		return FALSE;

	BOOL unhooked = UnhookWindowsHookEx(hook);
	if (unhooked)
	{
		s_hWndServer_LL = NULL;
		s_message_LL = 0;
	}
	return unhooked;
}

static LRESULT CALLBACK msghook_LL(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) // The specs say if nCode is < 0 then we just pass straight on.
	{
		CallNextHookEx(hook, nCode, wParam, lParam);
		return 0;
	}

	WPARAM newWParam = wParam;

	//KeyboardProcLParam l;
	//memset(&l, 0, sizeof(l));
	//l.lParam = (unsigned int)lParam;

	KBDLLHOOKSTRUCT *pHookStruct = (KBDLLHOOKSTRUCT*)lParam;

	//{
	//	LPWSTR str = new WCHAR[100];
	//	memset(str, 0, 100);
	//	wsprintf(str, L"LL: nCode=%i wparam=%#08x vCode=%#08x scanCode=%#08x time=%#08x flags=%#08x\n", nCode, wParam, pHookStruct->vkCode, pHookStruct->scanCode, pHookStruct->time, pHookStruct->flags);
	//	OutputDebugString(str);
	//	
	//}

	//LRESULT result;
	//result = ::SendMessage(s_hWndServer, s_message, newWParam, pHookStruct->vkCode);
	//if (result != 0)
	//{
	//	return -1;
	//}

	try
	{
		if (IsWindow(s_hWndServer_LL) == TRUE)
		{
			int repeatCount = 1;
			int scanCode = pHookStruct->scanCode & 0xFF;
			scanCode = scanCode << 16;
			int extended = pHookStruct->flags & 0x01;
			extended = extended << 24;
			int alt = (pHookStruct->flags & (1 << 5)) >> 5;
			alt = alt << 24;
			int transitionState = (pHookStruct->flags & (1 << 7)) >> 7;
			int previousState = transitionState;
			previousState = previousState << 30;
			transitionState = transitionState << 31;

			LPARAM newLParam = repeatCount | scanCode | extended | alt | previousState | transitionState;

			LRESULT result;
			//result = ::SendMessage(s_hWndServer_LL, s_message_LL, newWParam, pHookStruct->vkCode);
			result = ::SendMessage(s_hWndServer_LL, s_message_LL, pHookStruct->vkCode, newLParam);
			if (result != 0)
			{
				LPWSTR str = new WCHAR[100];
				memset(str, 0, 100);
				wsprintf(str, L"blocking LL: nCode=%i wparam=%#08x %#08x\n", nCode, wParam, lParam);
				OutputDebugString(str);
				return -1;
			}
		}
	} catch (...)
	{
	}

	//if (IsWindow(s_hWndServer_LL) == TRUE)
	//{
	//	::PostMessage(s_hWndServer_LL, s_message_LL, newWParam, pHookStruct->vkCode);
	//}

	return CallNextHookEx(hook, nCode, wParam, lParam);

}