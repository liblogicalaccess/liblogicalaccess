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
#include "islogkbdhooklib.hpp"

#include "islogkbdsettings.hpp"
#include "islogkbdlogs.hpp"

#pragma data_seg(".KEYBOARDHOOK")
HWND s_hWndServer = NULL;
UINT s_message    = 0;

HWND s_hWndServer_LL = NULL;
UINT s_message_LL    = 0;

#pragma data_seg()
#pragma comment(linker, "/section:.KEYBOARDHOOK,rws")

HMODULE __hKbdHookModule;

HHOOK hook;
HHOOK hook_LL;
static LRESULT CALLBACK msghook(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK msghook_LL(int nCode, WPARAM wParam, LPARAM lParam);

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("#islogkbdhooklib::DLL_PROCESS_ATTACH# called.");
        __hKbdHookModule = hModule;
        islogkbdlib::KbdSettings::getInstance()->Initialize();
        islogkbdlib::KbdLogs::getInstance()->Initialize();
        break;
    case DLL_PROCESS_DETACH:
        OutputDebugStringA("#islogkbdhooklib::DLL_PROCESS_DETACH# called.");
        if (s_hWndServer != NULL)
            ClearHook(s_hWndServer);
        islogkbdlib::KbdLogs::getInstance()->Uninitialize();
        islogkbdlib::KbdSettings::getInstance()->Uninitialize();
        break;
    }
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

union KeyboardProcLParam {
    unsigned int lParam;
    struct _keyboardProcLParam
    {
        unsigned short repeatCount : 16;
        unsigned short scanCode : 8;
        unsigned short extendedKey : 1;
        unsigned short reserved : 4;
        unsigned short contextCode : 1;
        unsigned short previousKeyState : 1;
        unsigned short transitionCode : 1;
    } values;
};

KEYBOARDHOOK_API BOOL SetHook(HWND hWnd, UINT message)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#SetHook# begins.");
    if (s_hWndServer != NULL)
        return FALSE; // already hooked

    hook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)msghook, __hKbdHookModule, 0);
    if (hook == FALSE)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#SetHook# Unable to set hook {%d}!", GetLastError());
        return FALSE;
    }

    s_hWndServer = hWnd;
    s_message    = message;
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#SetHook# Hook set successfully.");
    return TRUE;
}

/* Low level hook */
KEYBOARDHOOK_API BOOL SetHook_LL(HWND hWnd, UINT message)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#SetHook_LL# begins.");

    if (s_hWndServer_LL != NULL)
        return FALSE; // already hooked

    hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)msghook_LL, __hKbdHookModule, 0);
    if (hook == FALSE)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#SetHook_LL# Unable to set hook {%d}!", GetLastError());
        return FALSE;
    }

    s_hWndServer_LL = hWnd;
    s_message_LL    = message;
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#SetHook_LL# Hook set successfully.");
    return TRUE;
}

KEYBOARDHOOK_API BOOL ClearHook(HWND hWnd)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#ClearHook# begins.");
    if ((hWnd == NULL) || (s_hWndServer == NULL) || (hWnd != s_hWndServer))
        return FALSE;

    BOOL unhooked = UnhookWindowsHookEx(hook);
    if (unhooked)
    {
        s_hWndServer = NULL;
        s_message    = 0;
    }
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#ClearHook# returns {%d}", unhooked);
    return unhooked;
}

/* Low level hook */
KEYBOARDHOOK_API BOOL ClearHook_LL(HWND hWnd)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#ClearHook_LL# begins.");

    if ((hWnd == NULL) || (s_hWndServer_LL == NULL) || (hWnd != s_hWndServer_LL))
        return FALSE;

    BOOL unhooked = UnhookWindowsHookEx(hook);
    if (unhooked)
    {
        s_hWndServer_LL = NULL;
        s_message_LL    = 0;
    }
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#ClearHook_LL# returns {%d}",
                                                  unhooked);
    return unhooked;
}

static LRESULT CALLBACK msghook(int nCode, WPARAM wParam, LPARAM lParam)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook# begins.");

    if (nCode < 0) // The specs say if nCode is < 0 then we just pass straight on.
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#msghook# nCode < 0. Calling next hook...");
        CallNextHookEx(hook, nCode, wParam, lParam);
        return 0;
    }

    islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook# nCode=%i wparam=%#08x %#08x",
                                                  nCode, wParam, lParam);

    WPARAM newWParam = wParam;
    if (nCode == HC_NOREMOVE)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#msghook# nCode == HC_NOREMOVE. Calling next hook...");
        // I was originally sending HC_NOREMOVE message to my app, but they weren't
        // necessary and were causing problems
        // with debugging in visual studio.
        // newWParam = wParam | 0x80000000;

        CallNextHookEx(hook, nCode, wParam, lParam);
        return 0;
    }

    try
    {
        if (IsWindow(s_hWndServer) == TRUE)
        {
            islogkbdlib::KbdLogs::getInstance()->LogEvent(
                "#msghook# Sending message to window...");
            LRESULT result;
            result = ::SendMessage(s_hWndServer, s_message, newWParam, lParam);
            if (result != 0)
            {
                islogkbdlib::KbdLogs::getInstance()->LogEvent(
                    "#msghook# SendMessage error {%d}", result);
                return -1;
            }
        }
        else
        {
            islogkbdlib::KbdLogs::getInstance()->LogEvent(
                "#msghook# Error, server handle is not a window!");
        }
    }
    catch (std::exception &e)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook# Error occured {%s}!",
                                                      e.what());
    }
    catch (...)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#msghook# Unknown error occured {%d}", GetLastError());
    }

    islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook# Done. Calling next hook...");
    return CallNextHookEx(hook, nCode, wParam, lParam);
}

/* Low level hook */
static LRESULT CALLBACK msghook_LL(int nCode, WPARAM wParam, LPARAM lParam)
{
    islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook_LL# begins.");

    if (nCode < 0) // The specs say if nCode is < 0 then we just pass straight on.
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#msghook_LL# nCode < 0. Calling next hook...");
        CallNextHookEx(hook, nCode, wParam, lParam);
        return 0;
    }

    // WPARAM newWParam = wParam;

    // KeyboardProcLParam l;
    // memset(&l, 0, sizeof(l));
    // l.lParam = (unsigned int)lParam;

    KBDLLHOOKSTRUCT *pHookStruct = (KBDLLHOOKSTRUCT *)lParam;

    islogkbdlib::KbdLogs::getInstance()->LogEvent(
        "#msghook_LL# nCode=%i wparam=%#08x %#08x", nCode, wParam, lParam);

    //{
    //	LPWSTR str = new WCHAR[100];
    //	memset(str, 0, 100);
    //	wsprintf(str, L"LL: nCode=%i wparam=%#08x vCode=%#08x scanCode=%#08x time=%#08x
    // flags=%#08x\n", nCode, wParam, pHookStruct->vkCode, pHookStruct->scanCode,
    // pHookStruct->time, pHookStruct->flags);
    //	OutputDebugString(str);
    //
    //}

    // LRESULT result;
    // result = ::SendMessage(s_hWndServer, s_message, newWParam, pHookStruct->vkCode);
    // if (result != 0)
    //{
    //	return -1;
    //}

    try
    {
        if (IsWindow(s_hWndServer_LL) == TRUE)
        {
            islogkbdlib::KbdLogs::getInstance()->LogEvent(
                "#msghook_LL# Sending message to window...");

            int repeatCount     = 1;
            int scanCode        = pHookStruct->scanCode & 0xFF;
            scanCode            = scanCode << 16;
            int extended        = pHookStruct->flags & 0x01;
            extended            = extended << 24;
            int alt             = (pHookStruct->flags & (1 << 5)) >> 5;
            alt                 = alt << 24;
            int transitionState = (pHookStruct->flags & (1 << 7)) >> 7;
            int previousState   = transitionState;
            previousState       = previousState << 30;
            transitionState     = transitionState << 31;

            LPARAM newLParam =
                repeatCount | scanCode | extended | alt | previousState | transitionState;

            LRESULT result;
            // result = ::SendMessage(s_hWndServer_LL, s_message_LL, newWParam,
            // pHookStruct->vkCode);
            result = ::SendMessage(s_hWndServer_LL, s_message_LL, pHookStruct->vkCode,
                                   newLParam);
            if (result != 0)
            {
                islogkbdlib::KbdLogs::getInstance()->LogEvent(
                    "#msghook_LL# SendMessage error {%d}", result);
                return -1;
            }
        }
        else
        {
            islogkbdlib::KbdLogs::getInstance()->LogEvent(
                "#msghook_LL# Error, server handle is not a window!");
        }
    }
    catch (std::exception &e)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent("#msghook_LL# Error occured {%s}!",
                                                      e.what());
    }
    catch (...)
    {
        islogkbdlib::KbdLogs::getInstance()->LogEvent(
            "#msghook_LL# Unknown error occured {%d}", GetLastError());
    }

    // if (IsWindow(s_hWndServer_LL) == TRUE)
    //{
    //	::PostMessage(s_hWndServer_LL, s_message_LL, newWParam, pHookStruct->vkCode);
    //}

    islogkbdlib::KbdLogs::getInstance()->LogEvent(
        "#msghook_LL# Done. Calling next hook...");
    return CallNextHookEx(hook, nCode, wParam, lParam);
}