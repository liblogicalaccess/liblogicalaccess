// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Bug number 621653 reported on Microsoft Connect
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4005)
#include <intsafe.h>
#include <stdint.h>
#pragma warning (push)
#pragma warning (default : 4005)
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <winsock2.h>
#include <windows.h>

// TODO: reference additional headers your program requires here
//const CLSID CLSID_LibLOGICALACCESS		=	{ 0x288569B8, 0xFD41, 0x482e, 0x89, 0x21, 0x67, 0x15, 0xCA, 0x75, 0x28, 0x40 };
//const GUID GUID_TypeLibLOGICALACCESS		=	{ 0x57748055, 0x657d, 0x4365, 0x86, 0xD0, 0x10, 0x64, 0x85, 0xED, 0x05, 0xA2 };