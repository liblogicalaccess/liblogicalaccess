// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "logicalaccess/crypto/tomcrypt.h"

#include "logicalaccess/settings.h"
#include "logicalaccess/logs.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		register_cipher(&des_desc);
		register_cipher(&des3_desc);
#ifndef _DEBUG
		if (IsDebuggerPresent())
		{
			MessageBox(NULL, TEXT("You are using a debugging application, please read the following message from ISLOG.\n\nOur native binary files aren't protected by any compression, encryption, obfuscation, emulation or anti-debugging method. We truly believe these tricks decrease the software quality, and better concentrate our resources to code enhancement and user satisfaction.\nThe reverse engineering process is prohibed by the end user license agreement that you accepted at the software installation. However, if despite your commitment violation you continue your analyze and find any improvement or security flaw to fix, please report it to us freely.\nYou can also contact us to get more information about this binary, you would be surprise about how many information you can get by just asking for it. Greater than eating 4MB of ASM code, isn't it ? :)"),
				TEXT("Debugger notification from ISLOG"), MB_OK | MB_ICONINFORMATION);
		}
#endif

		LOGICALACCESS::Settings::getInstance().Initialize();  
		INFO_("Process attached !");  
		break;
	case DLL_PROCESS_DETACH:
		INFO_("Process detached !");
		LOGICALACCESS::Settings::getInstance().Uninitialize();  
		
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