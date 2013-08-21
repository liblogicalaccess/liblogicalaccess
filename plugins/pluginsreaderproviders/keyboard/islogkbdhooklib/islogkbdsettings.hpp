#ifndef ISLOGKBDHOOKLIB_SETTINGS_HPP
#define ISLOGKBDHOOKLIB_SETTINGS_HPP

#include "islogkbdhooklib.hpp"

#include <string>
#include <sstream>
#include <vector>

#ifdef _MSC_VER
#include <Windows.h>
extern HMODULE __hKbdHookModule;
#endif

namespace islogkbdlib
{
	class KEYBOARDHOOK_API KbdSettings
	{
		public:

			static KbdSettings* getInstance();
			
			void Initialize();  
			void Uninitialize();  

			bool IsLogEnabled;
			std::string LogFileName;

			std::string getDllPath();

		protected:

			KbdSettings();
			
			void LoadSettings();
			void SaveSettings();

			static KbdSettings* instance;
			
		private:
		
			void reset();
	};
}

#endif