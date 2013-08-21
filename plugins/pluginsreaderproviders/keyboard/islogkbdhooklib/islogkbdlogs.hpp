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

			void Initialize();  
			void Uninitialize(); 

			void LogEvent(const char *, ...);

		protected:

			KbdLogs();

			void OpenLogFile(const char* pszLogFile);
			void CloseLogFile();

			static KbdLogs* instance;

		private:

			static FILE* currentLogFile;
	};
}

#endif
