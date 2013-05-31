// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/crypto/tomcrypt.h"

#include "logicalaccess/settings.hpp"
#include "logicalaccess/logs.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		register_cipher(&des_desc);
		register_cipher(&des3_desc);

		logicalaccess::Settings::getInstance().Initialize();  
		INFO_("Process attached !");  
		break;
	case DLL_PROCESS_DETACH:
		INFO_("Process detached !");
		logicalaccess::Settings::getInstance().Uninitialize();  
		
		unregister_cipher(&des3_desc);
		unregister_cipher(&des_desc);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}