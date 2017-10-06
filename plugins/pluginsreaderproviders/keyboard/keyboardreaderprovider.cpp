/**
 * \file keyboardreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard reader provider.
 */

#include "keyboardreaderprovider.hpp"

#include <sstream>
#include <iomanip>
#include <codecvt>

#include <chrono>
#include <thread>

#ifdef _WINDOWS
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#include <Userenv.h>
#include <tlhelp32.h>
#endif

namespace logicalaccess
{
    KeyboardReaderProvider::KeyboardReaderProvider() :
        ReaderProvider()
    {
        LOG(LogLevel::INFOS) << "Creating new KeyboardReaderProvider instance...";

        hWatchThrd = NULL;
        watchSessions = false;
#ifdef _WINDOWS
        //sharedGuid = "test";
        createKbdFileMapping();
        createKbdEvent();
#endif
    }

    KeyboardReaderProvider::~KeyboardReaderProvider()
    {
        release();
    }

    void KeyboardReaderProvider::release()
    {
#ifdef _WINDOWS
        freeKbdEvent();
        freeKbdFileMapping();
        stopWatchingActiveConsole();
#endif
    }

    std::shared_ptr<KeyboardReaderProvider> KeyboardReaderProvider::getSingletonInstance()
    {
        LOG(LogLevel::INFOS) << "Getting singleton instance...";
        static std::shared_ptr<KeyboardReaderProvider> instance;
        if (!instance)
        {
            instance.reset(new KeyboardReaderProvider());
#ifdef _WINDOWS
            instance->startAndWatchOnActiveConsole();
#endif
            instance->refreshReaderList();
        }

        return instance;
    }

#ifdef _WINDOWS

    void KeyboardReaderProvider::generateSharedGuid()
    {
        GUID shGuid;
        CoCreateGuid(&shGuid);
        OLECHAR* bstrGuid;
        StringFromCLSID(shGuid, &bstrGuid);
        std::wstring wshname(bstrGuid);
        CoTaskMemFree(bstrGuid);

        sharedGuid.assign(wshname.begin(), wshname.end());
    }

    long KeyboardReaderProvider::createKbdFileMapping()
    {
        LOG(LogLevel::INFOS) << "Creating the file mapping...";
        long ret = 0;
        SECURITY_ATTRIBUTES sa;

        char sharedName[512];
        memset(sharedName, 0x00, sizeof(sharedName));
        if (sharedGuid.empty())
        {
            generateSharedGuid();
        }
        sprintf_s(sharedName, sizeof(sharedName), "%s%s", KEYBOARD_SHAREDDATA, sharedGuid.c_str());
        LOG(LogLevel::INFOS) << "File mapping with shared name {" << sharedName << "}";

        fillSecurityDescriptor(&sa, NULL);

        shKeyboard = CreateFileMapping(NULL, &sa, PAGE_READWRITE, 0, sizeof(KeyboardSharedStruct), sharedName);
        if (!shKeyboard)
        {
            ret = GetLastError();
            LOG(LogLevel::ERRORS) << "CreateFileMapping error {" << ret << "}";
            return ret;
        }
        sKeyboard = static_cast<KeyboardSharedStruct*>(MapViewOfFile(shKeyboard, FILE_MAP_ALL_ACCESS, 0, 0, 0));

        LOG(LogLevel::INFOS) << "Keyboard structure size {" << sizeof(sKeyboard) << "}";

        if (sa.lpSecurityDescriptor)
        {
            LocalFree(sa.lpSecurityDescriptor);
            sa.lpSecurityDescriptor = NULL;
        }

        return ret;
    }

    long KeyboardReaderProvider::createKbdEvent()
    {
        LOG(LogLevel::INFOS) << "Creating the event...";
        long ret = 0;

        char eventName[512];
        memset(eventName, 0x00, sizeof(eventName));

        if (sharedGuid.empty())
        {
            generateSharedGuid();
        }
        sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENT, sharedGuid.c_str());
        LOG(LogLevel::INFOS) << "Event named {" << eventName << "}";

        hKbdEvent = CreateEvent(0, TRUE, FALSE, eventName);
        if (!hKbdEvent)
        {
            ret = GetLastError();
            LOG(LogLevel::ERRORS) << "CreateEvent error {" << ret << "} for hKbdEvent";
        }

        memset(eventName, 0x00, sizeof(eventName));
        sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENTPROCESEED, sharedGuid.c_str());
        LOG(LogLevel::INFOS) << "Event named {" << eventName << "}";
        hKbdEventProcessed = CreateEvent(0, TRUE, FALSE, eventName);
        if (!hKbdEventProcessed)
        {
            ret = GetLastError();
            LOG(LogLevel::ERRORS) << "CreateEvent error {" << ret << "} for hKbdEventProcessed";
        }

        memset(eventName, 0x00, sizeof(eventName));
        sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_HOSTEVENT, sharedGuid.c_str());
        LOG(LogLevel::INFOS) << "Event named {" << eventName << "}";
        hHostEvent = CreateEvent(0, TRUE, FALSE, eventName);
        if (!hHostEvent)
        {
            ret = GetLastError();
            LOG(LogLevel::ERRORS) << "CreateEvent error {" << ret << "} for hHostEvent";
        }

        memset(eventName, 0x00, sizeof(eventName));
        sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_STILLALIVEEVENT, sharedGuid.c_str());
        LOG(LogLevel::INFOS) << "Event named {" << eventName << "}";
        hStillAliveEvent = CreateEvent(0, TRUE, FALSE, eventName);
        if (!hStillAliveEvent)
        {
            ret = GetLastError();
            LOG(LogLevel::ERRORS) << "CreateEvent error {" << ret << "} for hStillAliveEvent";
        }

        return ret;
    }

    void KeyboardReaderProvider::freeKbdFileMapping()
    {
        LOG(LogLevel::INFOS) << "Releasing file mapping...";
        if (shKeyboard)
        {
            CloseHandle(shKeyboard);

            shKeyboard = NULL;
            sKeyboard = NULL;
        }
    }

    void KeyboardReaderProvider::freeKbdEvent()
    {
        LOG(LogLevel::INFOS) << "Releasing kbd events...";
        if (hKbdEvent)
        {
            CloseHandle(hKbdEvent);
            hKbdEvent = NULL;
        }

        if (hKbdEventProcessed)
        {
            CloseHandle(hKbdEventProcessed);
            hKbdEventProcessed = NULL;
        }

        if (hHostEvent)
        {
            CloseHandle(hHostEvent);
            hHostEvent = NULL;
        }

        if (hStillAliveEvent)
        {
            CloseHandle(hStillAliveEvent);
            hStillAliveEvent = NULL;
        }
    }

    void KeyboardReaderProvider::fillSecurityDescriptor(LPSECURITY_ATTRIBUTES sa, PACL pACL)
    {
        PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (NULL == pSD)
        {
            LOG(LogLevel::ERRORS) << "LocalAlloc Error {" << GetLastError() << "}";
        }
        else
        {
            if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
            {
                LOG(LogLevel::ERRORS) << "InitializeSecurityDescriptor Error {" << GetLastError() << "}";
            }
            else
            {
                if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
                {
                    LOG(LogLevel::ERRORS) << "SetSecurityDescriptorDacl Error {" << GetLastError() << "}";
                }
                else
                {
                    sa->nLength = sizeof(SECURITY_ATTRIBUTES);
                    sa->bInheritHandle = FALSE;
                    sa->lpSecurityDescriptor = pSD;
                }
            }
        }
    }

    void KeyboardReaderProvider::startAndWatchOnActiveConsole()
    {
        watchSessions = true;
        hWatchThrd = CreateThread(NULL, 0, WatchThread, NULL, 0, NULL);
    }

    // Watch current session active console and start hook on it if it changed and not yet started for this session
    DWORD WINAPI WatchThread(LPVOID lpThreadParameter)
    {
        LOG(LogLevel::INFOS) << "WatchThread begins...";
        std::shared_ptr<KeyboardReaderProvider> readerProvider = KeyboardReaderProvider::getSingletonInstance();

        if (readerProvider != NULL)
        {
            DWORD lastSessionId = 0;
            bool isUserSession = true;
            DWORD hostSessionId = 0;
            DWORD currentProcessSessionID = GetCurrentProcessId();
            LOG(LogLevel::INFOS) << "Current process ID {" << hostSessionId << "}";

            if (ProcessIdToSessionId(currentProcessSessionID, &hostSessionId))
            {
                LOG(LogLevel::INFOS) << "Current process Session ID {" << hostSessionId << "}";

                // On windows XP Session 0 is ok, but on Vist and higher, this is not a valid user session.
                OSVERSIONINFO osvi;
                ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
                osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                GetVersionEx(&osvi);
                if (osvi.dwMajorVersion >= 6)
                {
                    LOG(LogLevel::INFOS) << "We are on windows Vista or Higher. Need to check for invalid session 0.";
                    isUserSession = (hostSessionId != 0);
                }
                else
                {
                    LOG(LogLevel::INFOS) << "We are on windows XP or less. Session 0 is valid.";
                    isUserSession = true;
                }
            }
            else
            {
                LOG(LogLevel::ERRORS) << "ProcessIdToSessionId Error {" << GetLastError() << "}";
            }

            if (isUserSession)
            {
                DWORD processId = readerProvider->launchHook();
                readerProvider->processHookedSessions.insert(readerProvider->processHookedSessions.end(), std::pair<DWORD, DWORD>(hostSessionId, processId));

                LOG(LogLevel::INFOS) << "User session ID valid. Session ID added to watch list.";
            }
            else
            {
                LOG(LogLevel::ERRORS) << "Not user session ID.";
            }

            do
            {
                if (!isUserSession)
                {
                    DWORD cSessionId = WTSGetActiveConsoleSessionId();

                    LOG(LogLevel::INFOS) << "WTSGetActiveConsoleSessionId = {" << cSessionId << "} vs Last session ID {" << lastSessionId << "}";

                    if (cSessionId != lastSessionId)
                    {
                        if (readerProvider->processHookedSessions.find(cSessionId) == readerProvider->processHookedSessions.end())
                        {
                            LOG(LogLevel::INFOS) << "New session ID detected {" << cSessionId << "}. Adding session to watch list...";
                            DWORD cProcessId = readerProvider->launchHookIntoDifferentSession(cSessionId);
                            readerProvider->processHookedSessions.insert(readerProvider->processHookedSessions.end(), std::pair<DWORD, DWORD>(cSessionId, cProcessId));
                        }

                        lastSessionId = cSessionId;
                    }
                }

                SetEvent(readerProvider->hStillAliveEvent);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } while (readerProvider->watchSessions);
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Reader provider is NULL.";
        }

        return 0;
    }

    void KeyboardReaderProvider::stopWatchingActiveConsole()
    {
        watchSessions = false;

        unsigned int timeout = 2000;
        LOG(LogLevel::INFOS) << "Waiting for listening thread to exit. Max timeout {" << timeout << "}...";

        if (hWatchThrd != NULL)
        {
            DWORD res = WaitForSingleObject(hWatchThrd, timeout);
            if (res == WAIT_OBJECT_0)
            {
                LOG(LogLevel::INFOS) << "Listening thread exited.";
            }
            else
            {
                LOG(LogLevel::ERRORS) << "Listening thread timeout! Killing it!";
                TerminateThread(hWatchThrd, 0);
                hWatchThrd = NULL;
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Thread handle is NULL!";
        }

        for (SessionHookMap::iterator it; it != processHookedSessions.end(); ++it)
        {
            LOG(LogLevel::INFOS) << "Terminating hook process {" << (*it).second << "}...";
            bool ret = terminateProcess((*it).second, 0);

            if (!ret)
            {
                LOG(LogLevel::INFOS) << "Unable to terminate process!";
            }
        }

        processHookedSessions.clear();
        LOG(LogLevel::INFOS) << "Everything has been stopped successfully!";
    }

    bool KeyboardReaderProvider::terminateProcess(DWORD dwProcessId, UINT uExitCode)
    {
        DWORD dwDesiredAccess = PROCESS_TERMINATE;
        BOOL  bInheritHandle = FALSE;
        HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
        if (hProcess == NULL)
            return FALSE;

        BOOL result = TerminateProcess(hProcess, uExitCode);

        CloseHandle(hProcess);

        return (result == TRUE);
    }

    HANDLE KeyboardReaderProvider::retrieveWinlogonUserToken(const DWORD destSessionId, const SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, const TOKEN_TYPE TokenType)
    {
        LOG(LogLevel::INFOS) << "RetrieveWinlogonUserToken begins";

        DWORD winlogonPid = 0;
        HANDLE hUserTokenDup = NULL, hPToken = NULL, hProcess = NULL;
        TOKEN_PRIVILEGES tp;
        LUID luid;
        PROCESSENTRY32 procEntry;

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE)
        {
            LOG(LogLevel::ERRORS) << "CreateToolhelp32Snapshot failed";
            return NULL;
        }

        procEntry.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hSnap, &procEntry))
        {
            LOG(LogLevel::ERRORS) << "Process32First failed";
            if (hSnap)
                CloseHandle(hSnap);
            return NULL;
        }

        LOG(LogLevel::INFOS) << "Finding the winlogon process in the right session ID !";
        do
        {
            if (_stricmp(procEntry.szExeFile, "winlogon.exe") == 0)
            {
                //Finding the winlogon process and make sure that it's running in the console session
                DWORD winlogonSessId = 0;
                if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId))
                {
                    LOG(LogLevel::INFOS) << "Current try - Winlogon in Session Id {" << winlogonSessId << "}";
                    if (winlogonSessId == destSessionId)
                    {
                        LOG(LogLevel::INFOS) << "Winlogon process found !";
                        winlogonPid = procEntry.th32ProcessID;
                        break;
                    }
                }
                else
                {
                    LOG(LogLevel::ERRORS) << "Unable to ProcessIdToSessionId Error {" << GetLastError() << "}";
                }
            }
        } while (Process32Next(hSnap, &procEntry));

        if (hSnap)
            CloseHandle(hSnap);

        LOG(LogLevel::INFOS) << "Opening Process";
        hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid);

        if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_READ, &hPToken))
        {
            LOG(LogLevel::ERRORS) << "Process token open Error: {" << std::hex << GetLastError() << std::dec << "}";
        }

        LOG(LogLevel::INFOS) << "Looking Privilege value";
        if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
        {
            LOG(LogLevel::ERRORS) << "Lookup Privilege value Error: {" << std::hex << GetLastError() << std::dec << "}";
        }
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        LOG(LogLevel::INFOS) << "Duplicating and setting privileges";

        DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, ImpersonationLevel, TokenType, &hUserTokenDup);
        SetTokenInformation(hUserTokenDup, TokenSessionId, (void*)destSessionId, sizeof(DWORD));

        if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL))
        {
            LOG(LogLevel::INFOS) << "Adjust Privilege value Error: {" << std::hex << GetLastError() << std::dec << "}";
        }

        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        {
            LOG(LogLevel::ERRORS) << "Token does not have the privilege";
        }

        //Perform All the Close Handles task
        if (hProcess)
            CloseHandle(hProcess);
        if (hPToken)
            CloseHandle(hPToken);

        return hUserTokenDup;
    }

    DWORD KeyboardReaderProvider::launchHookIntoDifferentSession(DWORD destSessionId)
    {
        LOG(LogLevel::INFOS) << "Begins for sessionID {" << destSessionId << "}";

        HANDLE hUserTokenDup = retrieveWinlogonUserToken(destSessionId, SecurityIdentification, TokenPrimary);
        if (hUserTokenDup == NULL)
        {
            LOG(LogLevel::ERRORS) << "Unable to get the USER TOKEN !!";
        }

        return launchHook(hUserTokenDup);
    }

    DWORD KeyboardReaderProvider::launchHook(HANDLE hUserTokenDup)
    {
        LOG(LogLevel::INFOS) << "Begins ...";

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        STARTUPINFO si;
        DWORD dwCreationFlags;
        LPVOID pEnv = NULL;

        std::string appName = getHookPath();
        std::string cmdLine = "\"" + appName + "\" " + getHookArguments();

        dwCreationFlags = REALTIME_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.lpDesktop = "winsta0\\default";

        if (hUserTokenDup != NULL)
        {
            if (CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE))
            {
                dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
            }
            else
            {
                pEnv = NULL;
            }
        }

        char* app = NULL;
        char* cmd = NULL;

        if (!appName.empty())
        {
            app = const_cast<char*>(appName.c_str());
            LOG(LogLevel::INFOS) << "Application name {" << app << "}";
        }

        if (!cmdLine.empty())
        {
            cmd = const_cast<char*>(cmdLine.c_str());
            LOG(LogLevel::INFOS) << "Command line {" << cmd << "}";
        }

        LOG(LogLevel::INFOS) << "Starting process...";

        if (hUserTokenDup != NULL)
        {
            if (!CreateProcessAsUser(hUserTokenDup, app, cmd, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi))
            {
                LOG(LogLevel::ERRORS) << "CreateProcessAsUser failed {" << GetLastError() << "}";
            }
            else
            {
                LOG(LogLevel::INFOS) << "CreateProcessAsUser successfully ended !";
            }
        }
        else
        {
            if (!CreateProcess(app, cmd, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi))
            {
                LOG(LogLevel::ERRORS) << "CreateProcess failed {" << std::hex << GetLastError() << std::dec << "}";
            }
            else
            {
                LOG(LogLevel::INFOS) << "CreateProcess successfully ended !";
            }
        }

        if (pEnv)
        {
            DestroyEnvironmentBlock(pEnv);
        }

        if (pi.hProcess)
            CloseHandle(pi.hProcess);
        if (pi.hThread)
            CloseHandle(pi.hThread);
        if (si.hStdOutput)
            CloseHandle(si.hStdOutput);
        if (si.hStdInput)
            CloseHandle(si.hStdInput);
        if (si.hStdError)
            CloseHandle(si.hStdError);
        if (hUserTokenDup)
            CloseHandle(hUserTokenDup);

        return pi.dwProcessId;
    }

    bool KeyboardReaderProvider::is64BitWindows() const
    {
#if defined(_WIN64)
        return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
        // 32-bit programs run on both 32-bit and 64-bit Windows
        // so must sniff
        BOOL f64 = FALSE;
        return (IsWow64Process(GetCurrentProcess(), &f64) && f64);
#else
        return false; // Win64 does not support Win16
#endif
    }

    std::string KeyboardReaderProvider::getHookPath() const
    {
        char curpath[MAX_PATH];
        memset(curpath, 0x00, sizeof(curpath));

        GetModuleFileName((HINSTANCE)&__ImageBase, curpath, sizeof(curpath));
        std::string::size_type pos = std::string(curpath).find_last_of("\\/");
        std::string path = std::string(curpath).substr(0, pos);

        if (is64BitWindows())
        {
            path += "\\islogkbdhook64.exe";
        }
        else
        {
            path += "\\islogkbdhook32.exe";
        }

        return path;
    }

    std::string KeyboardReaderProvider::getHookArguments() const
    {
        return std::string("-s " + sharedGuid);
    }
#endif

    std::shared_ptr<ReaderUnit> KeyboardReaderProvider::createReaderUnit()
    {
        LOG(LogLevel::INFOS) << "Creating new reader unit...";

        std::shared_ptr<KeyboardReaderUnit> ret;
        ret.reset(new KeyboardReaderUnit());
        ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

        return ret;
    }

    bool KeyboardReaderProvider::refreshReaderList()
    {
        LOG(LogLevel::INFOS) << "Refresh reader list...";
        d_readers.clear();

#ifdef _WINDOWS
        if (shKeyboard != NULL)
        {
            unsigned int timeout = 5000;
            LOG(LogLevel::INFOS) << "Waiting {" << timeout << "} milliseconds for host event set...";
            DWORD res = WaitForSingleObject(hHostEvent, timeout);
            if (res == WAIT_OBJECT_0)
            {
                //LOG(LogLevel::INFOS) << "Host event is set! A keyboard hook is started.");
                for (unsigned int i = 0; i < MAX_KEYBOARD_DEVICES; ++i)
                {
                    if (sKeyboard->devices[i].handle != NULL)
                    {
                        //LOG(LogLevel::INFOS) << "Keyboard[%u].name = {%s}", i, sKeyboard->devices[i].name);

                        if (strncmp(sKeyboard->devices[i].name, "", sizeof(sKeyboard->devices[i].name)))
                        {
                            std::shared_ptr<KeyboardReaderUnit> ru;
                            ru.reset(new KeyboardReaderUnit());
                            ru->setKeyboard(sKeyboard->devices[i].name, sKeyboard->devices[i].vendorId, sKeyboard->devices[i].productId);
                            ru->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

                            LOG(LogLevel::INFOS) << "Reader {" << sKeyboard->devices[i].name << "} added to the list.";

                            d_readers.push_back(ru);
                        }
                        else
                        {
                            LOG(LogLevel::ERRORS) << "Keyboard name is empty! Ignoring...";
                        }
                    }
                }
            }
            else
            {
                LOG(LogLevel::INFOS) << "Timeout reached when waiting for host event... No keyboard hook found.";
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "File mapping handle invalid...";
        }
#endif

        return true;
    }
}