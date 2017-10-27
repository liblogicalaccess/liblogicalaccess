// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/crypto/tomcrypt.h"

#include "logicalaccess/settings.hpp"
#include "logicalaccess/logs.hpp"

HMODULE __hLibLogicalAccessModule;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        __hLibLogicalAccessModule = hModule;
        DisableThreadLibraryCalls(hModule);

        register_cipher(&des_desc);
        register_cipher(&des3_desc);

        logicalaccess::Settings::getInstance()->Initialize();
        LOG(logicalaccess::LogLevel::INFOS) << "Process attached ! HMODULE: " << hModule;
        break;
    case DLL_PROCESS_DETACH:
        LOG(logicalaccess::LogLevel::INFOS) << "Process detached !";
        logicalaccess::Settings::getInstance()->Uninitialize();

        unregister_cipher(&des3_desc);
        unregister_cipher(&des_desc);
        break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    default:;
    }
    return TRUE;
}