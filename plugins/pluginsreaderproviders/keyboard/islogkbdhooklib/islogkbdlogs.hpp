#ifndef ISLOGKBDHOOKLIB_LOG_HPP
#define ISLOGKBDHOOKLIB_LOG_HPP

#include "islogkbdhooklib.hpp"

#include <stdio.h>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <string>
#include <wchar.h>

namespace islogkbdlib
{
	class KEYBOARDHOOK_API KbdLogs
	{
		public:

			static KbdLogs* getInstance();

			static void Initialize();  
			static void Uninitialize(); 

			static void LogEvent(const char *, ...);

		protected:

			KbdLogs();

			static void OpenLogFile(const char* pszLogFile);
			static void CloseLogFile();

			static KbdLogs* instance;

		private:

			static FILE* currentLogFile;
	};
}

#endif
