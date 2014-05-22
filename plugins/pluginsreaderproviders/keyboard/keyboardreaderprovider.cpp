/**
 * \file keyboardreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Keyboard reader provider.
 */

#include "keyboardreaderprovider.hpp"

#include <sstream>
#include <iomanip>
#include <codecvt>

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
		INFO_("Creating new KeyboardReaderProvider instance...");

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

	boost::shared_ptr<KeyboardReaderProvider> KeyboardReaderProvider::getSingletonInstance()
	{
		INFO_("Getting singleton instance...");
		static boost::shared_ptr<KeyboardReaderProvider> instance;
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
		INFO_("Creating the file mapping...");
		long ret = 0;
		SECURITY_ATTRIBUTES sa;

		char sharedName[512];
		memset(sharedName, 0x00, sizeof(sharedName));
		if (sharedGuid.empty())
		{
			generateSharedGuid();
		}
		sprintf_s(sharedName, sizeof(sharedName), "%s%s", KEYBOARD_SHAREDDATA, sharedGuid.c_str());
		INFO_("File mapping with shared name {%s}", sharedName);

		fillSecurityDescriptor(&sa, NULL);

		shKeyboard = CreateFileMapping(NULL, &sa, PAGE_READWRITE, 0, sizeof(KeyboardSharedStruct), sharedName);
		if (!shKeyboard)
		{
			ret = GetLastError();
			ERROR_("CreateFileMapping error {%d}", ret);
			return ret;
		}
		sKeyboard = (KeyboardSharedStruct *)MapViewOfFile(shKeyboard, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		INFO_("Keyboard structure size {%d}", sizeof(sKeyboard));

		if (sa.lpSecurityDescriptor)
		{
			LocalFree(sa.lpSecurityDescriptor);
			sa.lpSecurityDescriptor = NULL;
		}

		return ret;
	}

	long KeyboardReaderProvider::createKbdEvent()
	{
		INFO_("Creating the event...");
		long ret = 0;

		char eventName[512];
		memset(eventName, 0x00, sizeof(eventName));

		if (sharedGuid.empty())
		{
			generateSharedGuid();
		}
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENT, sharedGuid.c_str());
		INFO_("Event named {%s}", eventName);

		hKbdEvent = CreateEvent(0, TRUE, FALSE, eventName);
		if (!hKbdEvent)
		{
			ret = GetLastError();
			ERROR_("CreateEvent error {%d} for hKbdEvent", ret);
		}

		memset(eventName, 0x00, sizeof(eventName));
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_EVENTPROCESEED, sharedGuid.c_str());
		INFO_("Event named {%s}", eventName);
		hKbdEventProcessed = CreateEvent(0, TRUE, FALSE, eventName);
		if (!hKbdEventProcessed)
		{
			ret = GetLastError();
			ERROR_("CreateEvent error {%d} for hKbdEventProcessed", ret);
		}

		memset(eventName, 0x00, sizeof(eventName));
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_HOSTEVENT, sharedGuid.c_str());
		INFO_("Event named {%s}", eventName);
		hHostEvent = CreateEvent(0, TRUE, FALSE, eventName);
		if (!hHostEvent)
		{
			ret = GetLastError();
			ERROR_("CreateEvent error {%d} for hHostEvent", ret);
		}

		memset(eventName, 0x00, sizeof(eventName));
		sprintf_s(eventName, sizeof(eventName), "%s%s", KEYBOARD_STILLALIVEEVENT, sharedGuid.c_str());
		INFO_("Event named {%s}", eventName);
		hStillAliveEvent = CreateEvent(0, TRUE, FALSE, eventName);
		if (!hStillAliveEvent)
		{
			ret = GetLastError();
			ERROR_("CreateEvent error {%d} for hStillAliveEvent", ret);
		}

		return ret;
	}

	void KeyboardReaderProvider::freeKbdFileMapping()
	{
		INFO_("Releasing file mapping...");
		if (shKeyboard)
		{
			CloseHandle(shKeyboard);

			shKeyboard = NULL;
			sKeyboard = NULL;
		}
	}

	void KeyboardReaderProvider::freeKbdEvent()
	{
		INFO_("Releasing kbd events...");
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
		PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (NULL == pSD) 
		{ 
			ERROR_("LocalAlloc Error {%u}", GetLastError());
		}
		else
		{
			if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) 
			{
				ERROR_( "InitializeSecurityDescriptor Error {%u}", GetLastError());
			}
			else
			{
				if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
				{
					ERROR_("SetSecurityDescriptorDacl Error {%u}", GetLastError());
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
		INFO_("WatchThread begins...");
		boost::shared_ptr<KeyboardReaderProvider> readerProvider = KeyboardReaderProvider::getSingletonInstance();

		if (readerProvider != NULL)
		{
			DWORD lastSessionId = 0;
			bool isUserSession = true;
			DWORD hostSessionId = 0;
			DWORD currentProcessSessionID = GetCurrentProcessId();
			INFO_("Current process ID {%d}", hostSessionId);

			if (ProcessIdToSessionId(currentProcessSessionID, &hostSessionId))
			{
				INFO_("Current process Session ID {%d}", hostSessionId);

				// On windows XP Session 0 is ok, but on Vist and higher, this is not a valid user session.
				OSVERSIONINFO osvi;
				ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
				osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				GetVersionEx(&osvi);
				if (osvi.dwMajorVersion >= 6)
				{
					INFO_("We are on windows Vista or Higher. Need to check for invalid session 0.");
					isUserSession = (hostSessionId != 0);
				}
				else
				{
					INFO_("We are on windows XP or less. Session 0 is valid.");
					isUserSession = true;
				}
			}
			else
			{
				ERROR_("ProcessIdToSessionId Error {%u}", GetLastError());
			}

			if (isUserSession)
			{
				DWORD processId = readerProvider->launchHook();
				readerProvider->processHookedSessions.insert(readerProvider->processHookedSessions.end(), std::pair<DWORD, DWORD>(hostSessionId, processId));

				INFO_("User session ID valid. Session ID added to watch list.");
			}
			else
			{
				ERROR_("Not user session ID.");
			}

			do
			{
				if (!isUserSession)
				{
					DWORD cSessionId = WTSGetActiveConsoleSessionId();

					INFO_("WTSGetActiveConsoleSessionId = {%d} vs Last session ID {%d}", cSessionId, lastSessionId);

					if (cSessionId != lastSessionId)
					{
						if (readerProvider->processHookedSessions.find(cSessionId) == readerProvider->processHookedSessions.end())
						{
							INFO_("New session ID detected {%d}. Adding session to watch list...", cSessionId);
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
			ERROR_("Reader provider is NULL.");
		}

		return 0;
	}

	void KeyboardReaderProvider::stopWatchingActiveConsole()
	{
		watchSessions = false;

		unsigned int timeout = 2000;
		INFO_("Waiting for listening thread to exit. Max timeout {%u}...", timeout);
		
		if (hWatchThrd != NULL)
		{
			DWORD res = WaitForSingleObject(hWatchThrd, timeout);
			if (res == WAIT_OBJECT_0)
			{
				INFO_("Listening thread exited.");
			}
			else
			{
				ERROR_("Listening thread timeout! Killing it!");
				TerminateThread(hWatchThrd, 0);
				hWatchThrd = NULL;
			}
		}
		else
		{
			ERROR_("Thread handle is NULL!");
		}

		for (SessionHookMap::iterator it; it != processHookedSessions.end(); ++it)
		{
			INFO_("Terminating hook process {%d}...", (*it).second);
			bool ret = terminateProcess((*it).second, 0);
			
			if (!ret)
			{
				INFO_("Unable to terminate process!");
			}
		}

		processHookedSessions.clear();
		INFO_("Everything has been stopped successfully!");
	}

	bool KeyboardReaderProvider::terminateProcess(DWORD dwProcessId, UINT uExitCode)
	{
		DWORD dwDesiredAccess = PROCESS_TERMINATE;
		BOOL  bInheritHandle  = FALSE;
		HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
		if (hProcess == NULL)
			return FALSE;

		BOOL result = TerminateProcess(hProcess, uExitCode);

		CloseHandle(hProcess);

		return (result == TRUE);
	}

	HANDLE KeyboardReaderProvider::retrieveWinlogonUserToken(const DWORD destSessionId, const SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, const TOKEN_TYPE TokenType)
	{
		INFO_("RetrieveWinlogonUserToken begins");

		DWORD winlogonPid = 0;
		HANDLE hUserTokenDup = NULL, hPToken = NULL, hProcess = NULL;
		TOKEN_PRIVILEGES tp;
		LUID luid;
		PROCESSENTRY32 procEntry;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap == INVALID_HANDLE_VALUE)
		{
			ERROR_("CreateToolhelp32Snapshot failed");
			return NULL;
		}

		procEntry.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hSnap, &procEntry))
		{
			ERROR_("Process32First failed");
			if (hSnap)
				CloseHandle(hSnap);
			return NULL;
		}

		INFO_("Finding the winlogon process in the right session ID !");
		do
		{
			if (_stricmp(procEntry.szExeFile, "winlogon.exe") == 0)
			{
				//Finding the winlogon process and make sure that it's running in the console session
				DWORD winlogonSessId = 0;
				if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId))
				{
					INFO_("Current try - Winlogon in Session Id {%d}", winlogonSessId);
					if (winlogonSessId == destSessionId)
					{
						INFO_("Winlogon process found !");
						winlogonPid = procEntry.th32ProcessID;
						break;
					}
				}
				else
				{
					ERROR_("Unable to ProcessIdToSessionId Error {%d}", GetLastError());
				}
			}

		} while (Process32Next(hSnap, &procEntry));

		if (hSnap)
			CloseHandle(hSnap);

		INFO_("Opening Process");
		hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid);

		if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY
										|TOKEN_DUPLICATE|TOKEN_ASSIGN_PRIMARY|TOKEN_ADJUST_SESSIONID
										|TOKEN_READ|TOKEN_WRITE, &hPToken))
		{
			ERROR_("Process token open Error: {%u}\n", GetLastError()); 
		}

		INFO_("Looking Privilege value");
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		{
			ERROR_("Lookup Privilege value Error: {%u}\n", GetLastError()); 
		}
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		INFO_("Duplicating and setting privileges");

		DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, ImpersonationLevel, TokenType, &hUserTokenDup);
		SetTokenInformation(hUserTokenDup, TokenSessionId, (void*)destSessionId, sizeof(DWORD));

		if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL))
		{
			INFO_("Adjust Privilege value Error: %u\n", GetLastError()); 
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			ERROR_("Token does not have the privilege"); 
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
		INFO_("Begins for sessionID {%d}", destSessionId);

		HANDLE hUserTokenDup = retrieveWinlogonUserToken(destSessionId, SecurityIdentification, TokenPrimary);
		if (hUserTokenDup ==  NULL)
		{
			ERROR_("Unable to get the USER TOKEN !!");
		}

		return launchHook(hUserTokenDup);
	}

	DWORD KeyboardReaderProvider::launchHook(HANDLE hUserTokenDup)
	{
		INFO_("Begins ...");

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
			INFO_("Application name {%s}", app);
		}

		if (!cmdLine.empty())
		{
			cmd = const_cast<char*>(cmdLine.c_str());
			INFO_("Command line {%s}", cmd);
		}

		INFO_("Starting process...");

		if (hUserTokenDup != NULL)
		{
			if (!CreateProcessAsUser(hUserTokenDup, app, cmd, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi))
			{
				ERROR_("CreateProcessAsUser failed {%d}", GetLastError());
			}
			else
			{
				INFO_("CreateProcessAsUser successfully ended !");
			}
		}
		else
		{
			if (!CreateProcess(app, cmd, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi))
			{
				ERROR_("CreateProcess failed {%d}", GetLastError());
			}
			else
			{
				INFO_("CreateProcess successfully ended !");
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

	boost::shared_ptr<ReaderUnit> KeyboardReaderProvider::createReaderUnit()
	{
		INFO_("Creating new reader unit...");

		boost::shared_ptr<KeyboardReaderUnit> ret;
		ret.reset(new KeyboardReaderUnit());
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}	

	bool KeyboardReaderProvider::refreshReaderList()
	{
		INFO_("Refresh reader list...");
		d_readers.clear();		

#ifdef _WINDOWS
		if (shKeyboard != NULL)
		{
			unsigned int timeout = 5000;
			INFO_("Waiting {%u} milliseconds for host event set...", timeout);
			DWORD res = WaitForSingleObject(hHostEvent, timeout);
			if (res == WAIT_OBJECT_0)
			{
				//INFO_("Host event is set! A keyboard hook is started.");
				for (unsigned int i = 0; i < MAX_KEYBOARD_DEVICES; ++i)
				{
					if (sKeyboard->devices[i].handle != NULL)
					{
						//INFO_("Keyboard[%u].name = {%s}", i, sKeyboard->devices[i].name);

						if (strncmp(sKeyboard->devices[i].name, "", sizeof(sKeyboard->devices[i].name)))
						{
							boost::shared_ptr<KeyboardReaderUnit> ru;
							ru.reset(new KeyboardReaderUnit());
							ru->setKeyboard(sKeyboard->devices[i].name);
							ru->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

							INFO_("Reader {%s} added to the list.", sKeyboard->devices[i].name);

							d_readers.push_back(ru);
						}
						else
						{
							ERROR_("Keyboard name is empty! Ignoring...");
						}
					}
				}
			}
			else
			{
				INFO_("Timeout reached when waiting for host event... No keyboard hook found.");
			}
		}
		else
		{
			ERROR_("File mapping handle invalid...");
		}
#endif

		return true;
	}	
}

