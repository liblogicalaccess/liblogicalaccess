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

#ifndef _DEFINED_KEYBOARD_HOOK
#define _DEFINED_KEYBOARD_HOOK

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEYBOARDHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEYBOARDHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ISLOGKBDHOOKLIB_EXPORTS
#define KEYBOARDHOOK_API __declspec(dllexport)
#else
#define KEYBOARDHOOK_API __declspec(dllimport)
#endif

KEYBOARDHOOK_API BOOL SetHook(HWND hWnd, UINT message);
KEYBOARDHOOK_API BOOL ClearHook(HWND hWnd);

KEYBOARDHOOK_API BOOL SetHook_LL(HWND hWnd, UINT message);
KEYBOARDHOOK_API BOOL ClearHook_LL(HWND hWnd);

#ifdef __cplusplus
}
#endif

#endif